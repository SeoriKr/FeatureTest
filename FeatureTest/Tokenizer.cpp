#include <iostream>
#include <fstream>
#include <sstream>
#include "string"
#include "vector"

using namespace std;

void ParseOBJ(const string& filename);

struct FVector
{
	float x;
	float y;
	float z;
};

struct FVector2
{
	float u;
	float v;
};

int main()
{
	return 0;
}

void ParseOBJ(const string& filename)
{
	ifstream file(filename);

	if (!file.is_open())
	{
		cout << "Can't open file!" << endl;
		return;
	}

	vector<FVector> Vertices;
	vector<FVector2> TexCoords;
	vector<FVector> Normals;

	std::string line;
	int LineNumber = 1;

	while (getline(file, line))
	{
		if (line.empty())
		{
			cout << "ºó ÁÙ " << endl;
			LineNumber++;
			continue;
		}

		if (line[0] == '#') {
			std::cout << "ÁÖ¼®" << endl;
			LineNumber++;
			continue;
		}
	}

	stringstream ss(line);
	string type;
	ss >> type;

	if (type == "v")
	{
		FVector v;

		ss >> v.x >> v.y >> v.z;
		Vertices.push_back(v);
	}
	else if (type == "vt")
	{
		FVector2 vt;

		ss >> vt.u >> vt.v;
		TexCoords.push_back(vt);
	}
	else if (type == "vn")
	{
		FVector v;

		ss >> v.x >> v.y >> v.z;
		Normals.push_back(v);
	}
	else if (type == "f")
	{
		FVector v;

		ss >> v.x >> v.y >> v.z;
		Vertices.push_back(v);
	}
	


}