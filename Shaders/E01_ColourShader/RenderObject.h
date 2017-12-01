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
// Other
#include "RenderTextureCubemap.h"
#include <functional>


class RenderObject
{
public:
	RenderObject(BaseMesh* mesh, SkyboxShader* skyboxShader, ObjectShader* objectShader, DisplacementShader* displacementShader, GeometryShader* geometryShader, ColourShader* colourShader, DepthShader* depthShader, DepthTesselationShader* depthTesselationShader, BlurShader* blurShader, DepthOfFieldShader* DOFShader);
	~RenderObject();

	// Set
	inline void SetPosition(XMFLOAT3 newPosition) { position = newPosition; }
	inline void SetPosition(float x, float y, float z) { SetPosition(XMFLOAT3(x, y, z)); }
	inline void SetRotation(XMFLOAT3 newRotation) { rotation = newRotation; }
	inline void SetRotation(float x, float y, float z) { SetRotation(XMFLOAT3(x, y, z)); }
	inline void SetScale(XMFLOAT3 newScale) { scale = newScale; }
	inline void SetScale(float x, float y, float z) { SetScale(XMFLOAT3(x, y, z)); }

	inline void SetShadowMapQuality(XMFLOAT2 lshadowMapSize, float ldirShadowMapQuality, float lpointShadowMapQuality) {
		 shadowMapSize = lshadowMapSize;
		 dirShadowMapQuality = ldirShadowMapQuality;
		 pointShadowMapQuality = lpointShadowMapQuality;
	}
	inline void SetTesselationQuality(float lminTessFactor, float lmaxTessFactor, float lminTessDist, float lmaxTessDist){
		 minTessFactor = lminTessFactor;
		 maxTessFactor = lmaxTessFactor;
		 minTessDist = lminTessDist;
		 maxTessDist = lmaxTessDist;
	}
	inline bool IsUsingDynamicReflections() { return dynamicReflection; }
	inline void UseDynamicReflections(bool value) { dynamicReflection = value; }
	inline void SetStaticReflectionsTexture(ID3D11ShaderResourceView* txt) { skybox = txt; }

	// Get
	inline XMFLOAT3 GetPosition() { return position; }

	// Draw reflection cubemap
	void UpdateReflectionCubemap(D3D* renderer, std::function<void(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection, bool renderReflection)> renderScene);

	// Render using given shader
	void RenderSkybox(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);
	void RenderObjectShader(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, XMFLOAT3 mCol, float mMetallic, float mRoughness, bool useGGXDistribution, bool useGGXGeometry, bool renderReflections = true);
	void RenderDisplacement(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_envCubemap);
	void RenderGrass(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_noise_length, ID3D11ShaderResourceView* texture_noise_wind, float currentTime, float windStrength, float windFreq);	// Geometry shader
	void RenderTexture(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);	// Colour shader
	void RenderDepth(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange);
	void RenderTesselationDepth(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, float focalDistance, float focalRange, ID3D11ShaderResourceView* texture_displacement, XMFLOAT3 cameraPosition);
	void RenderBlur(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTextureFirstPass, ID3D11ShaderResourceView * sceneTextureSecondPass, ID3D11ShaderResourceView* depthTexture, float screenWidth, float screenHeight);
	void RenderDepthOfField(ID3D11DeviceContext* deviceContext, XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* depthTexture, ID3D11ShaderResourceView* blurTexture, float screenResX, float screenResY, bool useDOF, bool useVignette, bool useBW);	// Blur based on depth map

	bool dynamicReflection;

private:
	void SetWorldMatrix(XMMATRIX& world);

private:
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;

	BaseMesh* mesh;
	RenderTextureCubemap* reflectionCubemap;
	ID3D11ShaderResourceView* skybox;	// For static reflections

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


	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Information passed to shaders
	// Shadows
	XMFLOAT2 shadowMapSize;
	float dirShadowMapQuality;
	float pointShadowMapQuality;
	// Tesselation
	float minTessFactor;
	float maxTessFactor;
	float minTessDist;
	float maxTessDist;
};

