#pragma once

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class SkyboxShader : public BaseShader
{

public:

	SkyboxShader(ID3D11Device* device, HWND hwnd);
	~SkyboxShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11RasterizerState* rasterizerState;
	ID3D11DepthStencilState* stencilState;
};

