// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "SkyboxMesh.h"
#include "ColourShader.h"
#include "SkyboxShader.h"
#include "ObjectShader.h"
#include "DepthShader.h"
#include "BlurShader.h"

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
	void renderFinalPostProcessing();
	void renderGUITexture();
	void renderScene();
	void gui();

private:
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, orthoViewMatrix;
	float screenWidth, screenHeight;

	SkyboxShader* skyboxShader;
	ObjectShader* objectShader;	// Uses dwarf.obj, use head.obj later to show tesellation on

	BaseMesh* skyboxMesh;	// Zmienic pozniej zeby nie renderowalo 2 stron skoro i tak tylko 1 widac bedzie
	BaseMesh* floorMesh;
	BaseMesh* objectMesh;

	Light* pointLight;

	// Render texture - post processing
	ColourShader* colourShader;
	DepthShader* depthShader;
	BlurShader* blurShader;
	RenderTexture* depthTexture;
	RenderTexture* sceneTexture;
	RenderTexture* blurTexture;
	OrthoMesh* orthoMesh;

	
};

#endif