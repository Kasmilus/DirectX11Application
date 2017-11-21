#pragma once

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class RenderTextureCubemap
{
public:
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	RenderTextureCubemap(ID3D11Device* device, float screenNear, float screenDepth);
	~RenderTextureCubemap();

	void setRenderTarget(ID3D11DeviceContext* deviceContext, int cubemapFace);
	void clearRenderTarget(ID3D11DeviceContext* deviceContext, int cubemapFace, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* getShaderResourceView();

	XMMATRIX getProjectionMatrix();
	XMMATRIX getOrthoMatrix();


private:
	ID3D11Texture2D* renderTargetTexture;
	ID3D11RenderTargetView* renderTargetViews[6];
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT viewport;
	XMMATRIX projectionMatrix;
	XMMATRIX orthoMatrix;
};

