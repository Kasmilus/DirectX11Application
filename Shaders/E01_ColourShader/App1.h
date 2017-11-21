// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
// Include Meshes
#include "MyBaseMesh.h"
#include "SkyboxMesh.h"
#include "MyTesselationPlane.h"
#include "MyModelMesh.h"
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
#include "RenderObject.h"
#include "RenderTextureCubemap.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in);

	bool frame();

protected:
	bool render();
	void renderSphereCubemap();
	void renderShadowMapToTexture();
	void renderDepthToTexture();
	void renderSceneToTexture();
	void renderSceneToScreen();	// used when wire frame is on
	void renderFinalPostProcessing();
	void renderGUITexture();
	void renderScene();
	void gui();
	void ControlScene();

private:
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, orthoViewMatrix;
	float screenWidth, screenHeight;

	
	// objects
	RenderObject* skybox;
	RenderObject* floor;
	RenderObject* dwarf;
	RenderObject* sphere;
	RenderObject* grass;
	RenderObject* orthoMeshObj;

	// Meshes
	BaseMesh* skyboxMesh;	// Zmienic pozniej zeby nie renderowalo 2 stron skoro i tak tylko 1 widac bedzie
	MyBaseMesh* floorMesh;
	MyBaseMesh* sphereMesh;
	MyBaseMesh* objectMesh;
	BaseMesh* grassGeometryMesh;

	Light* pointLight;

	// Shaders
	SkyboxShader* skyboxShader;
	ObjectShader* objectShader;
	DisplacementShader* displacementShader;	// Using height map
	GeometryShader* grassGeometryShader;

	ColourShader* colourShader;
	DepthShader* depthShader;
	DepthTesselationShader* depthTesselationShader;
	BlurShader* blurShader;
	DepthOfFieldShader* DOFShader;

	// Render textures
	RenderTexture* depthTexture;
	RenderTexture* sceneTextureCurrent;
	RenderTexture* blurTextureDownSampled;
	RenderTexture* blurTextureUpSampled;
	RenderTexture* DOFTexture;
	RenderTexture* shadowMapTexture;
	RenderTextureCubemap* sphereReflectionCubemap;

	OrthoMesh* orthoMesh;

	float currentTime;
	float windFreq;
	float windStrength;

	// Depth of field focus variables
	float focalDistance = 0.48f;
	float focalRange = 2.56f;
	float focalDistanceChangeSpeed = 0.5f;
	float focalRangeChangeSpeed = 2.0f;

	bool postProcessingOn = false;
	bool wasPKeyDownLastFrame = false;
	bool wasIKeyDownLastFrame = false;

};

#endif