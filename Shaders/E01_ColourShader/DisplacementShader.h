#pragma once

#include "../DXFramework/BaseShader.h"
#include "../DXFramework/Light.h"

using namespace std;
using namespace DirectX;


class DisplacementShader : public BaseShader
{
public:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};
	struct LightBufferType
	{
		XMFLOAT4 diffuseColour;
		XMFLOAT4 ambientColour;
		XMFLOAT3 lightPosition;
		float padding;
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

	DisplacementShader(ID3D11Device* device, HWND hwnd);
	~DisplacementShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, Light* light, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_shadow);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR* vsFilename, WCHAR* psFilename);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tessellationBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateClampPoint;	// for sampling shadow map
	ID3D11SamplerState* sampleStateComparison;	// for filtering shadows
};