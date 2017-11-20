#pragma once

#include "../DXFramework/DXF.h"

// Include Shaders
#include "ColourShader.h"
#include "SkyboxShader.h"
#include "ObjectShader.h"
#include "DepthShader.h"
#include "BlurShader.h"
#include "DepthOfField.h"
#include "DisplacementShader.h"
#include "DepthTesselationShader.h"
#include "GeometryShader.h"

class RenderObject
{
public:
	RenderObject(BaseMesh* mesh, SkyboxShader* skyboxShader, ObjectShader* objectShader, DisplacementShader* displacementShader, GeometryShader* geometryShader, ColourShader* colourShader, DepthShader* depthShader, DepthTesselationShader* depthTesselationShader, BlurShader* blurShader, DepthOfFieldShader* DOFShader);
	~RenderObject();

	void SetPosition(XMFLOAT3 newPosition) { position = newPosition; }
	void SetPosition(float x, float y, float z) { SetPosition(XMFLOAT3(x, y, z)); }
	void SetRotation(XMFLOAT3 newRotation) { rotation = newRotation; }
	void SetRotation(float x, float y, float z) { SetRotation(XMFLOAT3(x, y, z)); }
	void SetScale(XMFLOAT3 newScale) { scale = newScale; }
	void SetScale(float x, float y, float z) { SetScale(XMFLOAT3(x, y, z)); }

	// Render using given shader
	void RenderSkybox(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);
	void RenderObjectShader(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, Light* light, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness);
	void RenderDisplacement(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, Light* light, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_shadow);
	void RenderGrass(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_noise_length, ID3D11ShaderResourceView* texture_noise_wind, float currentTime, float windStrength, float windFreq);	// Geometry shader
	void RenderTexture(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);	// Colour shader
	void RenderDepth(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange);
	void RenderTesselationDepth(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange, ID3D11ShaderResourceView* texture_displacement, XMFLOAT3 cameraPosition);
	void RenderBlur(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTextureFirstPass, ID3D11ShaderResourceView * sceneTextureSecondPass, ID3D11ShaderResourceView* depthTexture, float screenWidth, float screenHeight);
	void RenderDepthOfField(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* depthTexture, ID3D11ShaderResourceView* blurTexture, float screenResX, float screenResY);	// Blur based on depth map

private:
	void SetWorldMatrix(XMMATRIX& world);
	
private:
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;

	BaseMesh* mesh;

	// Shaders
	SkyboxShader* skyboxShader;
	ObjectShader* objectShader;
	DisplacementShader* displacementShader;	// Using height map
	GeometryShader* geometryShader;
	ColourShader* colourShader;
	DepthShader* depthShader;
	DepthTesselationShader* depthTesselationShader;
	BlurShader* blurShader;
	DepthOfFieldShader* DOFShader;
};

