// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	//BaseApplication::BaseApplication();
	skyboxMesh = nullptr;
	floorMesh = nullptr;
	objectMesh = nullptr;

	colourShader = nullptr;
	skyboxShader = nullptr;
	objectShader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in);

	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	// Initialise texture manager
	textureManagerCubemap = new TextureManagerCubemap(renderer->getDevice(), renderer->getDeviceContext());
	textureManagerCubemap->loadCubemapTexture("default", L"../res/brick1.dds");

	// MESHES
	// Skybox
	skyboxMesh = new SkyboxMesh(renderer->getDevice(), renderer->getDeviceContext());
	// Floor
	floorMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	// Object
	objectMesh = new Model(renderer->getDevice(), renderer->getDeviceContext(), "../res/dwarf.obj");

	// Textures
	textureMgr->loadTexture("object_base", L"../res/PBR/RustedIron/rustediron-streaks_basecolor.png");
	textureMgr->loadTexture("object_normal", L"../res/PBR/RustedIron/rustediron-streaks_normal.png");
	textureMgr->loadTexture("object_metallic", L"../res/PBR/RustedIron/rustediron-streaks_metallic.png");
	textureMgr->loadTexture("object_roughness", L"../res/PBR/RustedIron/rustediron-streaks_roughness.png");

	textureMgr->loadTexture("brick_base", L"../res/PBR/BlocksRough/blocksrough_basecolor.png");
	textureMgr->loadTexture("brick_normal", L"../res/PBR/BlocksRough/blocksrough_normal.png");
	textureMgr->loadTexture("brick_metallic", L"../res/PBR/BlocksRough/blocksrough_metallic.png");
	textureMgr->loadTexture("brick_roughness", L"../res/PBR/BlocksRough/blocksrough_roughness.png");

	textureManagerCubemap->loadCubemapTexture("skybox", L"../res/skybox.dds");

	// SHADERS
	//colourShader = new ColourShader(renderer->getDevice(), hwnd);
	skyboxShader = new SkyboxShader(renderer->getDevice(), hwnd);
	objectShader = new ObjectShader(renderer->getDevice(), hwnd);

	// LIGHTS
	pointLight = new Light();
	pointLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	pointLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	pointLight->setPosition(-1.5f, -1.0f, -0.2f);


	// Render to texture
	// OrthoMesh and shader set for different renderTarget
	sceneTextureCurrent = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	depthTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	blurTextureDownSampled = new RenderTexture(renderer->getDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	blurTextureUpSampled = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	DOFTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);


	// ortho size and position set based on window size
	// 200x200 pixels (standard would be matching window size for fullscreen mesh
	// Position default at 0x0 centre window, to offset change values (pixel)
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 1024, 576, 0, 0);
	colourShader = new ColourShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	blurShader = new BlurShader(renderer->getDevice(), hwnd);
	DOFShader = new DepthOfFieldShader(renderer->getDevice(), hwnd);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (skyboxMesh)
	{
		delete skyboxMesh;
		skyboxMesh = 0;
	}
	if (floorMesh)
	{
		delete floorMesh;
		floorMesh = 0;
	}
	if (objectMesh)
	{
		delete objectMesh;
		objectMesh = 0;
	}

	// Shaders
	if (colourShader)
	{
		delete colourShader;
		colourShader = 0;
	}
	if (skyboxShader)
	{
		delete skyboxShader;
		skyboxShader = 0;
	}
	if (objectShader)
	{
		delete objectShader;
		objectShader = 0;
	}

	if (textureManagerCubemap)
	{
		delete textureManagerCubemap;
		textureManagerCubemap = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Handle input
	ControlScene();

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	camera->update();

	renderDepthToTexture();
	renderSceneToTexture();

	renderScene();

	return true;
}

void App1::renderDepthToTexture()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	// Set the render target to be the render to texture.
	depthTexture->setRenderTarget(renderer->getDeviceContext());
	// Clear the render to texture.
	depthTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);
	// Get the world, view, and projection matrices from the camera and d3d objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();
	
	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	objectMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, focalDistance, focalRange);
	depthShader->render(renderer->getDeviceContext(), objectMesh->getIndexCount());
	// Render floor
	worldMatrix = XMMatrixTranslation(-50, -5, -20);
	floorMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, focalDistance, focalRange);
	depthShader->render(renderer->getDeviceContext(), floorMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::renderSceneToTexture()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Set the render target to be the render to texture.
	sceneTextureCurrent->setRenderTarget(renderer->getDeviceContext());

	// Clear the render to texture.
	sceneTextureCurrent->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	// Render object
	objectMesh->sendData(renderer->getDeviceContext());
	objectShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, camera->getPosition(), pointLight, textureMgr->getTexture("object_base"), textureMgr->getTexture("object_normal"), textureMgr->getTexture("object_metallic"), textureMgr->getTexture("object_roughness"));
	objectShader->render(renderer->getDeviceContext(), objectMesh->getIndexCount());

	// Render floor
	worldMatrix = XMMatrixTranslation(-50, -5, -20);
	floorMesh->sendData(renderer->getDeviceContext());
	objectShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, camera->getPosition(), pointLight, textureMgr->getTexture("brick_base"), textureMgr->getTexture("brick_normal"), textureMgr->getTexture("brick_metallic"), textureMgr->getTexture("brick_roughness"));
	objectShader->render(renderer->getDeviceContext(), floorMesh->getIndexCount());

	// Render skybox
	worldMatrix = XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	skyboxMesh->sendData(renderer->getDeviceContext());
	skyboxShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureManagerCubemap->getCubemapTexture("skybox"));
	skyboxShader->render(renderer->getDeviceContext(), skyboxMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::renderFinalPostProcessing()
{
	// Blur down sampled texture
	blurTextureDownSampled->setRenderTarget(renderer->getDeviceContext());
	blurTextureDownSampled->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	orthoMesh->sendData(renderer->getDeviceContext());
	// Horizontal blur
	blurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTextureCurrent->getShaderResourceView(), blurTextureDownSampled->getShaderResourceView(), screenWidth, screenHeight, true);
	blurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	// Vertical blur
	blurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, blurTextureDownSampled->getShaderResourceView(), blurTextureDownSampled->getShaderResourceView(), screenWidth, screenHeight, false);
	blurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Up sample
	blurTextureUpSampled->setRenderTarget(renderer->getDeviceContext());
	blurTextureUpSampled->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	orthoMesh->sendData(renderer->getDeviceContext());
	colourShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, blurTextureDownSampled->getShaderResourceView());
	colourShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Depth of field
	DOFTexture->setRenderTarget(renderer->getDeviceContext());
	DOFTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	orthoMesh->sendData(renderer->getDeviceContext());
	DOFShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTextureCurrent->getShaderResourceView(), depthTexture->getShaderResourceView(), blurTextureUpSampled->getShaderResourceView());
	DOFShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setBackBufferRenderTarget();
}

void App1::renderScene()
{
	renderFinalPostProcessing();

	//// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	// Get the world, view, and projection matrices from the camera and d3d objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Render GUI texture
	renderGUITexture();

	// Render GUI
	//gui();

	//// Present the rendered scene to the screen.
	renderer->endScene();
}

void App1::renderGUITexture() {
	// To render ortho mesh

	// pick texture to render
	ID3D11ShaderResourceView* targetResourceView = nullptr;
	if (postProcessingOn)
	{
		targetResourceView = DOFTexture->getShaderResourceView();
	}
	else
	{
		targetResourceView = sceneTextureCurrent->getShaderResourceView();
	}

	// ortho matrix for 2D rendering
	orthoMatrix = renderer->getOrthoMatrix();
	orthoViewMatrix = camera->getOrthoViewMatrix();

	// Turn off the Z buffer to begin all 2D rendering.
	renderer->setZBuffer(false);

	orthoMesh->sendData(renderer->getDeviceContext());
	colourShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, targetResourceView);
	colourShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
}

void App1::gui()
{
	// Force turn off on Geometry shader
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());

	// Render UI
	ImGui::Render();
}

void App1::ControlScene()
{
	// Post processing controls
	if (postProcessingOn)
	{
		float d = focalDistanceChangeSpeed * timer->getTime();
		if (input->isKeyDown('L'))
		{
			focalDistance += d;

			if (focalDistance > 2.0f)
				focalDistance = 2.0f;
		}
		if (input->isKeyDown('K'))
		{
			focalDistance -= d;

			if (focalDistance < -0.99f)
				focalDistance = -0.99f;
		}
		d = focalRangeChangeSpeed * timer->getTime();
		if (input->isKeyDown('M'))
		{
			focalRange += d;

			if (focalRange > 6)
				focalRange = 6;
		}
		if (input->isKeyDown('N'))
		{
			focalRange -= d;
			if (focalRange < 0.55f)
				focalRange = 0.55f;
		}
	}
	// Toggle post processing
	if (input->isKeyDown('P') && !wasPKeyDownLastFrame)
	{
		postProcessingOn = !postProcessingOn;
	}
	wasPKeyDownLastFrame = input->isKeyDown('P');
}

