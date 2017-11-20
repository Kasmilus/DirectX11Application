#pragma once

#include "MyBaseShader.h"

using namespace std;
using namespace DirectX;


class DepthTesselationShader : public MyBaseShader
{

public:

	struct DepthOfFieldBufferType
	{
		float focalDistance;
		float focalRange;
		XMFLOAT2 padding;
	};
	struct CameraBufferType
	{
		XMFLOAT3 position;
		float padding;
	};
	struct TessellationBufferType
	{
		float minTesselationDistance;
		float maxTesselationDistance;
		float minTesselationFactor;
		float maxTesselationFactor;
	};

	DepthTesselationShader(ID3D11Device* device, HWND hwnd);
	~DepthTesselationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange, ID3D11ShaderResourceView* texture_displacement, XMFLOAT3 cameraPosition);
	void render(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	void initShader(WCHAR*, WCHAR*);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* DepthOfFieldBuffer;
	ID3D11Buffer* tessellationBuffer;
	ID3D11Buffer* cameraBuffer;

};

