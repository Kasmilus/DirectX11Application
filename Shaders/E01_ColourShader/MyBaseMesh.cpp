#include "MyBaseMesh.h"

MyBaseMesh::MyBaseMesh()
{
}

// Release base objects (index, vertex buffers and texture object.
MyBaseMesh::~MyBaseMesh()
{
	if (indexBuffer)
	{
		indexBuffer->Release();
		indexBuffer = 0;
	}

	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = 0;
	}
}

int MyBaseMesh::getIndexCount()
{
	return indexCount;
}

// Sends geometry data to the GPU. Default primitive topology is TriangleList.
// To render alternative topologies this function needs to be overwritten.
void MyBaseMesh::sendData(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}



