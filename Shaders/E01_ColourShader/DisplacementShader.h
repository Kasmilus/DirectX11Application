#pragma once

#include "../DXFramework/BaseShader.h"
#include "MyLight.h"
#include <vector>

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
	struct PointLight
	{
		XMFLOAT4 lightDiffuseColour;
		XMFLOAT4 lightAmbientColour;
		XMFLOAT4 lightPositionAndRadius;
		XMFLOAT3 lightAttenuation;
		float isActive = 0;	// 0 - no light 1 - there is a light
	};
	struct LightBufferType
	{
		PointLight pointLight[4];
		// Directional
		XMFLOAT4 lightDiffuseColour;
		XMFLOAT4 lightAmbientColour;
		XMFLOAT3 lightDirection;
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
	struct ShadowMapBufferType
	{
		XMFLOAT2 shadowMapSize;
		float directionalShadowMapQuality; // 0 - no shadows, 1 - hard shadows, 2 - soft shadows
		float pointShadowMapQuality;
	};
	struct MaterialBufferType
	{
		XMFLOAT4 materialColour;
		float materialRoughness;
		float materialMetallic;
		XMFLOAT2 padding;
	};

	DisplacementShader(ID3D11Device* device, HWND hwnd);
	~DisplacementShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_envCubemap, float minTessFactor, float maxTessFactor, float minTessDist, float maxTessDist, XMFLOAT2 shadowMapSize, float dirShadowMapQuality, float pointShadowMapQuality);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR* vsFilename, WCHAR* psFilename);
	void initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tessellationBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* shadowMapBuffer;
	ID3D11Buffer* materialBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateClampPoint;	// for sampling shadow map
	ID3D11SamplerState* sampleStateComparison;	// for filtering shadows
};