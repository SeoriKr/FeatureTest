#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "string"

#include "map"

using namespace std;

struct FVector
{
	float x;
	float y;
	float z;

	FVector(float InX, float InY, float InZ) : x(InX), y(InY), z(InZ) {};
};

struct FVector2
{
	float u;
	float v;
};

struct Vertex
{
	FVector Location;
	FVector2 TexCoord;
	FVector Normal;
};

struct FStaticMesh
{
	vector<FVector> Locations;
	vector<FVector2> TexCoords;
	vector<FVector> Normals;
	vector<vector<FVector>> Faces;
};

struct UStaticMesh
{
	vector<Vertex> Vertices;
	vector<int> Indices;
};