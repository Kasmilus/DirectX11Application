#pragma once

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{

public:

	// Stores information about where to blur the screen 
	struct DepthOfFieldBufferType
	{
		float focalDistance;
		float focalRange;
		XMFLOAT2 padding;
	};

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* DepthOfFieldBuffer;

};

