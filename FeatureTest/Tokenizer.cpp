#include "Structs.h"

void ParseOBJ(const string& filename, FStaticMesh& OutFStaticMesh);
FVector ParseFaceVertex(const string& VertexData);
void Triangulate(const vector<FVector>& InFace, vector<vector<FVector>>& OutFaces);
void BuildStaticMesh(const FStaticMesh& InFStaticMesh, UStaticMesh& OutUStaticMesh);
void ShowUSMInfo(const UStaticMesh& InUStaticMesh);

int main()
{
	FStaticMesh FSM1, FSM2, FSM3;
	UStaticMesh USM1, USM2, USM3;

	ParseOBJ("Data/cube-tex.obj", FSM1);
	BuildStaticMesh(FSM1, USM1);
	ShowUSMInfo(USM1);

	ParseOBJ("Data/bitten_apple_mid.obj", FSM2);
	BuildStaticMesh(FSM2, USM2);
	ShowUSMInfo(USM2);

	ParseOBJ("Data/apple_mid.obj", FSM3);
	BuildStaticMesh(FSM3, USM3);
	ShowUSMInfo(USM3);

	return 0;
}

void ParseOBJ(const string& filename, FStaticMesh& OutFStaticMesh)
{
	ifstream file(filename);

	if (!file.is_open())
	{
		cout << "Can't open file!" << endl;
		return;
	}

	std::string line;
	int LineNumber = 1;

	while (getline(file, line))
	{
		if (line.empty())
		{
			cout << "빈 줄 " << endl;
			LineNumber++;
			continue;
		}

		if (line[0] == '#') {
			std::cout << "주석" << endl;
			LineNumber++;
			continue;
		}

		stringstream ss(line);
		string type;
		ss >> type;

		if (type == "v")
		{
			FVector v;

			ss >> v.x >> v.y >> v.z;
			OutFStaticMesh.Locations.push_back(v);
		}
		else if (type == "vt")
		{
			FVector2 vt;

			ss >> vt.u >> vt.v;
			OutFStaticMesh.TexCoords.push_back(vt);
		}
		else if (type == "vn")
		{
			FVector v;

			ss >> v.x >> v.y >> v.z;
			OutFStaticMesh.Normals.push_back(v);
		}
		else if (type == "f")
		{
			vector<FVector> Face;
			vector<vector<FVector>> Faces;
			string vertexData;

			while (ss >> vertexData) {
				FVector FaceVertex = ParseFaceVertex(vertexData);
				Face.push_back(FaceVertex);
			}

			Triangulate(Face, OutFStaticMesh.Faces);
		}
	}

	std::cout << "=== OBJ Parsing (Raw Data -> FStaticMesh) 요약 ===" << std::endl;
	std::cout << "총 정점: " << OutFStaticMesh.Locations.size() << "개" << std::endl;
	std::cout << "총 텍스처 좌표: " << OutFStaticMesh.TexCoords.size() << "개" << std::endl;
	std::cout << "총 법선: " << OutFStaticMesh.Normals.size() << "개" << std::endl;
	std::cout << "총 삼각형: " << OutFStaticMesh.Faces.size() << "개" << std::endl;
}

FVector ParseFaceVertex(const string& VertexData)
{
	FVector fv = { -1, -1, -1 };

	std::stringstream ss(VertexData);
	std::string token;
	int index = 0;

	// "/" 기준으로 분리: "1/2/3" -> "1", "2", "3"
	while (std::getline(ss, token, '/'))
	{
		if (!token.empty()) 
		{
			// 0에서 시작하도록 변환
			int value = std::stoi(token);

			if (index == 0)
			{ 
				fv.x = value - 1;
			}
			else if (index == 1)
			{
				fv.y = value - 1;
			}
			else if (index == 2)
			{ 
				fv.z = value - 1;
			}
		}
		index++;
	}

	return fv;
}

void Triangulate(const vector<FVector>& InFace, vector<vector<FVector>>& OutFaces)
{
	// 최소 3개의 정점이 필요
	if (InFace.size() < 3)
	{
		return;
	}

	// 이미 삼각형이면 그대로 추가
	if (InFace.size() == 3)
	{
		OutFaces.push_back(InFace);
		return;
	}

	// Fan Triangulation: 첫 번째 정점을 중심으로 삼각형 생성
	// 예: [v0, v1, v2, v3] -> [v0,v1,v2], [v0,v2,v3]
	for (size_t i = 1; i < InFace.size() - 1; i++) {
		vector<FVector> triangle;
		triangle.push_back(InFace[0]);      // 첫 번째 정점 (중심)
		triangle.push_back(InFace[i]);      // 현재 정점
		triangle.push_back(InFace[i + 1]);  // 다음 정점
		OutFaces.push_back(triangle);
	}
}

void BuildStaticMesh(const FStaticMesh& InFStaticMesh, UStaticMesh& OutUStaticMesh)
{
	map<string, int> VertexMap;

	OutUStaticMesh.Vertices.clear();
	OutUStaticMesh.Indices.clear();
		
	for (const auto& Triangle : InFStaticMesh.Faces)
	{
		for (const auto& FaceVertex : Triangle)
		{
			string key = to_string((int)FaceVertex.x) + "/" + to_string((int)FaceVertex.y) + "/" + to_string((int)FaceVertex.z);

			auto it = VertexMap.find(key);

			if (it != VertexMap.end())
			{
				OutUStaticMesh.Indices.push_back(it->second);
			}
			else
			{
				Vertex NewVertex;

				int vIdx = (int)FaceVertex.x;
				{
					if (vIdx >= 0 && vIdx < InFStaticMesh.Locations.size())
					{
						NewVertex.Location = InFStaticMesh.Locations[vIdx];
					}
					else
					{
						cout << "Vertex location index is out of range: " << vIdx << endl;
						return;
					}
				}

				int vtIdx = (int)FaceVertex.y;
				{
					if (vtIdx >= 0 && vtIdx < InFStaticMesh.TexCoords.size())
					{
						NewVertex.TexCoord = InFStaticMesh.TexCoords[vtIdx];
					}
					else
					{
						cout << "Vertex UV index is out of range: " << vtIdx << endl;
						return;
					}
				}

				int vnIdx = (int)FaceVertex.z;
				{
					if (vnIdx >= 0 && vnIdx < InFStaticMesh.Normals.size())
					{
						NewVertex.Normal = InFStaticMesh.Normals[vnIdx];
					}
					else
					{
						cout << "Vertex normal index is out of range: " << vnIdx << endl;
						return;
					}
				}

				int NewIndex = OutUStaticMesh.Vertices.size();
				VertexMap[key] = NewIndex;

				OutUStaticMesh.Vertices.push_back(NewVertex);
				OutUStaticMesh.Indices.push_back(NewIndex);
			}
		}
	}
}

void ShowUSMInfo(const UStaticMesh& InUStaticMesh)
{
	std::cout << "=== StaticMesh Translation (FStaticMesh -> UStaticMesh) 요약 ===" << std::endl;
	std::cout << "총 정점 수  : " << InUStaticMesh.Vertices.size() << "개" << std::endl;
	std::cout << "총 인덱스 수: " << InUStaticMesh.Indices.size() << "개" << std::endl;
}