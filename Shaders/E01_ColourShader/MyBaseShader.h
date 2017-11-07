#pragma once

/*
	just like base shader but is loading also tangent and binormal vectors into the vertex shader
*/

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <fstream>
#include "../DXFramework/imgui.h"

using namespace std;
using namespace DirectX;


class MyBaseShader
{
protected:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	MyBaseShader(ID3D11Device* device, HWND hwnd);
	~MyBaseShader();

	virtual void render(ID3D11DeviceContext* deviceContext, int vertexCount);

protected:
	virtual void initShader(WCHAR*, WCHAR*) = 0;
	virtual void loadVertexShader(WCHAR* filename);
	void loadHullShader(WCHAR* filename);
	void loadDomainShader(WCHAR* filename);
	void loadGeometryShader(WCHAR* filename);
	void loadPixelShader(WCHAR* filename);

protected:
	ID3D11Device* renderer;
	HWND hwnd;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11HullShader* hullShader;
	ID3D11DomainShader* domainShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11InputLayout* layout;
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
};