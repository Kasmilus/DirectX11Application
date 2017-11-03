#pragma once

/*
Edited base mesh class so it's storing now tangent and binormal vectors
*/

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class MyBaseMesh
{
protected:

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

public:
	MyBaseMesh();
	~MyBaseMesh();

	virtual void sendData(ID3D11DeviceContext* deviceContext);
	int getIndexCount();

protected:
	virtual void initBuffers(ID3D11Device*) = 0;

	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int vertexCount, indexCount;
};