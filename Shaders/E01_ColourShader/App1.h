// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "TextureManagerCubemap.h"
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

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in);

	bool frame();

protected:
	bool render();
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

	// Framework extensions
	TextureManagerCubemap* textureManagerCubemap;

	SkyboxShader* skyboxShader;
	ObjectShader* objectShader;	// Uses dwarf.obj, use head.obj later to show tessellation on
	DisplacementShader* floorShader;	// Using height map

	BaseMesh* skyboxMesh;	// Zmienic pozniej zeby nie renderowalo 2 stron skoro i tak tylko 1 widac bedzie
	MyBaseMesh* floorMesh;
	MyBaseMesh* objectMesh;

	Light* pointLight;

	// Render texture - post processing
	ColourShader* colourShader;
	DepthShader* depthShader;
	BlurShader* blurShader;
	DepthOfFieldShader* DOFShader;

	RenderTexture* depthTexture;
	RenderTexture* sceneTextureCurrent;
	RenderTexture* blurTextureDownSampled;
	RenderTexture* blurTextureUpSampled;
	RenderTexture* DOFTexture;

	OrthoMesh* orthoMesh;

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