#pragma once

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class BlurShader : public BaseShader
{

public:

	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		float isHorizontal;	// 0 - false(vertical), 1 - true(horizontal)
		float padding;
	};

	BlurShader(ID3D11Device* device, HWND hwnd);
	~BlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* depthTexture, float screenWidth, float screenHeight, bool isHorizintal);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* screenSizeBuffer;
	ID3D11RasterizerState* rasterizerState;
	ID3D11DepthStencilState* stencilState;
};

