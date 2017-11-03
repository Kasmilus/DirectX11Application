#pragma once

#include "MyBaseMesh.h"
#include "../DXFramework/TokenStream.h"
#include <vector>
#include <fstream>

using namespace DirectX;

class MyModelMesh : public MyBaseMesh
{
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
	};

public:
	MyModelMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
	~MyModelMesh();

protected:
	void initBuffers(ID3D11Device* device);
	void loadModel(char* filename);
	void calculateTangentAndBinormal(ModelType vertex1, ModelType vertex2, ModelType vertex3, XMFLOAT3& tangent, XMFLOAT3& binormal);

	ModelType* model;
};
