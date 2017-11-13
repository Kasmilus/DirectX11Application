#pragma once

/*
	Shader applying depth of field and vignette effects
*/

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class DepthOfFieldShader : public BaseShader
{

public:
	struct ScreenSizeBufferType
	{
		XMFLOAT2 screenResolution;
		XMFLOAT2 padding;
	};

	DepthOfFieldShader(ID3D11Device* device, HWND hwnd);
	~DepthOfFieldShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* depthTexture, ID3D11ShaderResourceView* blurTexture, float screenResX, float screenResY);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* screenSizeBuffer;

};

