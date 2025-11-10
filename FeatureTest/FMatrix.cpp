#include <memory>

#include "Structs.h"

struct FMatrix
{
	float M[4][4];

	FMatrix()
	{
		memset(M, 0, sizeof(M));
		M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1.0f;
	}

	static FMatrix Identity()
	{
		return FMatrix();
	}

	static FMatrix Zero()
	{
		FMatrix Result;

		memset(Result.M, 0, sizeof(Result.M));

		return Result;
	}

	// 루프 버전
	//FMatrix Transpose() const
	//{
	//	FMatrix Result;
	//
	//	for (int i = 0; i < 4; i++)
	//	{
	//		for (int j = 0; j < 4; j++)
	//		{
	//			// if 없는 게 더 빠르지 않을까라는 생각 (if (i!= j))
	//			Result.M[i][j] = M[j][i];
	//		}
	//	}
	//}

	// 언롤링 버전
	FMatrix Transpose() const {
		FMatrix Result;

		Result.M[0][0] = M[0][0]; Result.M[0][1] = M[1][0]; Result.M[0][2] = M[2][0]; Result.M[0][3] = M[3][0];
		Result.M[1][0] = M[0][1]; Result.M[1][1] = M[1][1]; Result.M[1][2] = M[2][1]; Result.M[1][3] = M[3][1];
		Result.M[2][0] = M[0][2]; Result.M[2][1] = M[1][2]; Result.M[2][2] = M[2][2]; Result.M[2][3] = M[3][2];
		Result.M[3][0] = M[0][3]; Result.M[3][1] = M[1][3]; Result.M[3][2] = M[2][3]; Result.M[3][3] = M[3][3];

		return Result;
	}

	float Minor(int row, int col) const
	{
		float SubMatrix[3][3];
		int i_sub = 0;

		for (int i = 0; i < 4; i++)
		{
			if (i == row)
			{
				continue;
			}

			int j_sub = 0;

			for (int j = 0; j < 4; j++)
			{
				if (j == col)
				{
					continue;
				}

				SubMatrix[i_sub][j_sub] = M[i][j];
				j_sub++;
			}

			i_sub++;
		}

		return SubMatrix[0][0] * (SubMatrix[1][1] * SubMatrix[2][2] - SubMatrix[1][2] * SubMatrix[2][1])
			 - SubMatrix[0][1] * (SubMatrix[1][0] * SubMatrix[2][2] - SubMatrix[1][2] * SubMatrix[2][0])
			 + SubMatrix[0][2] * (SubMatrix[1][0] * SubMatrix[2][1] - SubMatrix[1][1] * SubMatrix[2][0]);
	}
	
	float Cofactor(int row, int col) const
	{
		float minor = Minor(row, col);
		float sign = ((row + col) % 2 == 0) ? 1.0f : -1.0f;

		return sign * minor;
	}

	float Determinant() const
	{
		float det = 0.0f;

		for (int col = 0; col < 4; col++)
		{
			det += M[0][col] * Cofactor(0, col);
		}

		return det;
	}

	FMatrix Adjugate() const
	{
		FMatrix adj;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				adj.M[j][i] = Cofactor(i, j);
			}
		}

		return adj;
	}

	FMatrix Inverse() const
	{
		FMatrix Result;
		float det = Determinant();

		if (fabs(det) < 1e-6f)
		{
			return Identity();
		}

		FMatrix adj = Adjugate();

		float InvDet = 1.0f / det;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				Result.M[i][j] = InvDet * adj.M[i][j];
			}
		}

		return Result;
	}

	FVector operator*(const FVector& V) const
	{
		return FVector
		(
			V.x * M[0][0] + V.x * M[0][1] + V.y * M[0][2] + V.z * M[0][3],
			V.x * M[1][0] + V.x * M[1][1] + V.y * M[1][2] + V.z * M[1][3],
			V.x * M[2][0] + V.x * M[2][1] + V.y * M[2][2] + V.z * M[2][3]
		);
	}

	FMatrix operator*(const FMatrix& Other) const
	{
		FMatrix Result = FMatrix::Zero();

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					Result.M[i][j] += M[i][k] * Other.M[k][j];
				}
			}
		}

		return Result;
	}

	void ShowMatrix()
	{
		cout << "Determinant = " << Determinant() << endl << endl;
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				cout << i << " 행 " << j << " 열 = " << M[i][j];
				cout << " ";
			}

			cout << endl;
		}
	}
};

int main()
{
	FMatrix Mat;

	Mat.M[0][0] = 2.0f;  Mat.M[0][1] = 0.5f;  Mat.M[0][2] = 0.0f;  Mat.M[0][3] = 1.0f;
	Mat.M[1][0] = 0.5f;  Mat.M[1][1] = 2.0f;  Mat.M[1][2] = 0.0f;  Mat.M[1][3] = 2.0f;
	Mat.M[2][0] = 0.0f;  Mat.M[2][1] = 0.0f;  Mat.M[2][2] = 3.0f;  Mat.M[2][3] = 3.0f;
	Mat.M[3][0] = 0.0f;  Mat.M[3][1] = 0.0f;  Mat.M[3][2] = 0.0f;  Mat.M[3][3] = 1.0f;

	Mat.ShowMatrix();

	cout << endl << endl;

	FMatrix InvMat = Mat.Inverse();

	InvMat.ShowMatrix();

	cout << endl << endl;

	FMatrix Mul = Mat * InvMat;

	Mul.ShowMatrix();

	return 0;
}