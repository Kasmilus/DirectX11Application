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
	// Default values for controlling shaders
	currentTime = 0;
	// Wind
	windStrength = 0.45f;
	windFreq = 0.2f;
	// Tessellation
	minTessFactor = 1;
	maxTessFactor = 4;
	minTessDist = 50;
	maxTessDist = 5;
	// Depth of field
	focalDistance = 0.77f;
	focalRange = 0.55f;
	focalDistanceChangeSpeed = 0.5f;
	focalRangeChangeSpeed = 2.0f;
	// Shadow maps quality
	shadowMapSize = XMFLOAT2(1024, 1024);
	dirShadowMapQuality = 2;
	pointShadowMapQuality = 2;

	// TEXTURES
	textureMgr->loadTexture("object_base", L"../res/PBR/RustedIron/rustediron-streaks_basecolor.png");
	textureMgr->loadTexture("object_normal", L"../res/PBR/RustedIron/rustediron-streaks_normal.png");
	textureMgr->loadTexture("object_metallic", L"../res/PBR/RustedIron/rustediron-streaks_metallic.png");
	textureMgr->loadTexture("object_roughness", L"../res/PBR/RustedIron/rustediron-streaks_roughness.png");

	textureMgr->loadTexture("brick_base", L"../res/PBR/BlocksRough/blocksrough_basecolor.png");
	textureMgr->loadTexture("brick_normal", L"../res/PBR/BlocksRough/blocksrough_normal.png");
	textureMgr->loadTexture("brick_metallic", L"../res/PBR/BlocksRough/blocksrough_metallic.png");
	textureMgr->loadTexture("brick_roughness", L"../res/PBR/BlocksRough/blocksrough_roughness.png");
	textureMgr->loadTexture("brick_height", L"../res/PBR/BlocksRough/blocksrough_height.png");

	textureMgr->loadTexture("grass_base", L"../res/grass.png");
	textureMgr->loadTexture("grass_noise_length", L"../res/noise1.png");
	textureMgr->loadTexture("grass_noise_wind", L"../res/noise2.png");

	textureMgr->loadTexture("skybox", L"../res/skybox.dds");

	// MESHES
	skyboxMesh = new SkyboxMesh(renderer->getDevice(), renderer->getDeviceContext());
	floorMesh = new MyTesselationPlane(renderer->getDevice(), renderer->getDeviceContext(), 25);
	objectMesh = new MyModelMesh(renderer->getDevice(), renderer->getDeviceContext(), "../res/dwarf.obj");
	sphereMesh = new MyModelMesh(renderer->getDevice(), renderer->getDeviceContext(), "../res/sphere.obj");
	grassGeometryMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 50);

	// SHADERS
	skyboxShader = new SkyboxShader(renderer->getDevice(), hwnd);
	objectShader = new ObjectShader(renderer->getDevice(), hwnd);
	displacementShader = new DisplacementShader(renderer->getDevice(), hwnd);
	grassGeometryShader = new GeometryShader(renderer->getDevice(), hwnd);

	// LIGHTS
	directionalLight = new MyLight(renderer);
	directionalLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight->setPosition(-20.0f, 20, -5.0f);
	directionalLight->setDirection(0.7f, -0.25f, 0.4f);
	directionalLight->SetDirectional(true);

	MyLight* light = new MyLight(renderer);
	light->setDiffuseColour(2.0f, 0.0f, 0.0f, 1.0f);
	light->setPosition(1.5f, -2.0f, -2.2f);
	light->SetRadius(8);
	light->SetAttenuation(1, 0.5f, 0.75f);
	light->SetDirectional(false);
	light->SetIsActive(false);
	lights.push_back(light);
	MyLight* light2 = new MyLight(renderer);
	light2->setDiffuseColour(0.0f, 5.0f, 0.0f, 1.0f);
	light2->setPosition(1.5f, 2.0f, 3.2f);
	light2->SetRadius(9);
	light2->SetAttenuation(1, 0.5f, 1.45f);
	light2->SetDirectional(false);
	light2->SetIsActive(false);
	lights.push_back(light2);
	MyLight* light3 = new MyLight(renderer);
	light3->setDiffuseColour(0.0f, 0.0f, 6.0f, 1.0f);
	light3->setPosition(-8.5f, 0.25f, 5.2f);
	light3->SetRadius(6);
	light3->SetAttenuation(1, 0.1f, 1.45f);
	light3->SetDirectional(false);
	light3->SetIsActive(false);
	lights.push_back(light3);
	MyLight* light4 = new MyLight(renderer);
	light4->setDiffuseColour(8.0f, 8.0f, 0.0f, 1.0f);
	light4->setPosition(2, 4.0f, 1);
	light4->SetRadius(5);
	light4->SetAttenuation(1, 0.3f, 0);
	light4->SetDirectional(false);
	light4->SetIsActive(false);
	lights.push_back(light4);

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
	depthTesselationShader = new DepthTesselationShader(renderer->getDevice(), hwnd);

	// Render objects
	skybox = new RenderObject(skyboxMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	floor = new RenderObject(floorMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	floor->SetPosition(-15, -5, -10);
	floor->SetScale(1.2f, 0.25f, 1.2f);
	//floor->SetPosition(-1, -5, -10);
	//floor->SetScale(0.1f, 0.1f, 0.1f);
	dwarf = new RenderObject(objectMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	sphere = new RenderObject(sphereMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	sphere->SetPosition(-5, 0, 0);
	grass = new RenderObject(grassGeometryMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	grass->SetPosition(-25.5f, -5.2f, 14.0f);
	grass->SetScale(0.3f, 0.3f, 0.3f);
	orthoMeshObj = new RenderObject(orthoMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);

	// Set defualt shader params for objects
	floor->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	dwarf->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	sphere->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	floor->SetTesselationQuality(minTessFactor, maxTessFactor, minTessDist, maxTessDist);
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
	if (grassGeometryMesh)
	{
		delete grassGeometryMesh;
		grassGeometryMesh = 0;
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
	if (displacementShader)
	{
		delete displacementShader;
		displacementShader = 0;
	}
	if (depthShader)
	{
		delete depthShader;
		depthShader = 0;
	}
	if (depthTesselationShader)
	{
		delete depthTesselationShader;
		depthTesselationShader = 0;
	}
	if (grassGeometryShader)
	{
		delete grassGeometryShader;
		grassGeometryShader = 0;
	}

	// Render objects
	if (skybox)
	{
		delete skybox;
		skybox = 0;
	}
	if (floor)
	{
		delete floor;
		floor = 0;
	}
	if (dwarf)
	{
		delete dwarf;
		dwarf = 0;
	}
	if (grass)
	{
		delete grass;
		grass = 0;
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

	currentTime += timer->getTime();

	// Handle input
	ControlScene();
	// Update camera, lights
	UpdateObjects();
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
	// get world view projection matrices
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Render GUI texture
	if (renderer->getWireframeState())
	{
		renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
		renderScene(worldMatrix, viewMatrix, projectionMatrix);
	}
	else
	{
		// Shadow map
		renderShadowMaps();

		// Scene
		renderDepthToTexture();
		renderSceneToTexture();

		worldMatrix = renderer->getWorldMatrix();
		viewMatrix = camera->getViewMatrix();
		projectionMatrix = renderer->getProjectionMatrix();

		renderPostProcessingToTexture();

		renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
		renderSceneWithPostProcessing();	// Post processing
	}

	gui();	// GUI
	renderer->endScene();

	return true;
}

// Renders depth from each light's point of view(single ortho for directional, cubemap projection for point lights)
void App1::renderShadowMaps()
{
	directionalLight->UpdateShadowMap(renderer, std::bind(&App1::renderShadowMapScene, this, _1, _2, _3));
	for each (MyLight* light in lights)
	{
		light->UpdateShadowMap(renderer, std::bind(&App1::renderShadowMapScene, this, _1, _2, _3));
	}
	
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::renderShadowMapScene(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)
{
	// Dwarf
	dwarf->RenderDepth(renderer->getDeviceContext(), world, view, projection, focalDistance, focalRange);
	// Sphere
	sphere->RenderDepth(renderer->getDeviceContext(), world, view, projection, focalDistance, focalRange);
	// Floor
	floor->RenderTesselationDepth(renderer->getDeviceContext(), world, view, projection, focalDistance, focalRange, textureMgr->getTexture("brick_height"), camera->getPosition());
}

// Renders depth from camera point of view
void App1::renderDepthToTexture()
{
	depthTexture->setRenderTarget(renderer->getDeviceContext());
	depthTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	renderShadowMapScene(worldMatrix, viewMatrix, projectionMatrix);

	renderer->setBackBufferRenderTarget();
}

void App1::renderSphereCubemap()
{
	// Render reflection cubemap(pass function pointer)
	skybox->SetPosition(sphere->GetPosition());
	sphere->UpdateReflectionCubemap(renderer, std::bind(&App1::renderScene, this, _1, _2, _3));
	skybox->SetPosition(camera->getPosition());

	renderer->setBackBufferRenderTarget();
}

void App1::renderSceneToTexture()
{
	// Render reflection maps
	renderSphereCubemap();

	// Render the scene
	sceneTextureCurrent->setRenderTarget(renderer->getDeviceContext());
	sceneTextureCurrent->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	renderScene(worldMatrix, viewMatrix, projectionMatrix);

	renderer->setBackBufferRenderTarget();
}
// Render objects without any post-processing effects
void App1::renderScene(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)
{
	// Render dwarf
	dwarf->RenderObjectShader(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("object_base"), textureMgr->getTexture("object_normal"), textureMgr->getTexture("object_metallic"), textureMgr->getTexture("object_roughness"), textureMgr->getTexture("skybox"));
	// Render sphere
	sphere->RenderObjectShader(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("object_base"), textureMgr->getTexture("object_normal"), textureMgr->getTexture("object_metallic"), textureMgr->getTexture("object_roughness"));
	// Render floor
	floor->RenderDisplacement(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("brick_base"), textureMgr->getTexture("brick_normal"), textureMgr->getTexture("brick_metallic"), textureMgr->getTexture("brick_roughness"), textureMgr->getTexture("brick_height"), textureMgr->getTexture("skybox"));
	// Render grass
	grass->RenderGrass(renderer->getDeviceContext(), world, view, projection, textureMgr->getTexture("grass_base"), textureMgr->getTexture("grass_noise_length"), textureMgr->getTexture("grass_noise_wind"), currentTime, windStrength, windFreq);
	// Render skybox
	skybox->RenderSkybox(renderer->getDeviceContext(), world, view, projection, textureMgr->getTexture("skybox"));
}

void App1::renderPostProcessingToTexture()
{
	// Blur down sampled texture
	blurTextureDownSampled->setRenderTarget(renderer->getDeviceContext());
	blurTextureDownSampled->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	// Gaussian blur
	orthoMeshObj->RenderBlur(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTextureCurrent->getShaderResourceView(), blurTextureDownSampled->getShaderResourceView(), blurTextureDownSampled->getShaderResourceView(), screenWidth, screenHeight);

	// Up sample
	blurTextureUpSampled->setRenderTarget(renderer->getDeviceContext());
	blurTextureUpSampled->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	orthoMeshObj->RenderTexture(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, blurTextureDownSampled->getShaderResourceView());

	// Depth of field
	DOFTexture->setRenderTarget(renderer->getDeviceContext());
	DOFTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	orthoMeshObj->RenderDepthOfField(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTextureCurrent->getShaderResourceView(), depthTexture->getShaderResourceView(), blurTextureUpSampled->getShaderResourceView(), screenWidth, screenHeight);

	renderer->setBackBufferRenderTarget();
}

void App1::renderSceneWithPostProcessing() {
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
	//targetResourceView = directionalLight->GetShadowResourceView();
	// ortho matrix for 2D rendering
	orthoMatrix = renderer->getOrthoMatrix();
	orthoViewMatrix = camera->getOrthoViewMatrix();

	// Turn off the Z buffer to begin all 2D rendering.
	renderer->setZBuffer(false);

	orthoMeshObj->RenderTexture(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, targetResourceView);

	renderer->setZBuffer(true);
}

void App1::gui()
{
	// Force turn off on Geometry shader
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f\n", timer->getFPS());
	ImGui::Text("Toggle wire frame mode: I\n");
	ImGui::Text("Lighting: \n");
	ImGui::Text("	Move point light: <-Z X->\n");
	ImGui::Text("Toggle post-processing effects: P\n");
	if (postProcessingOn)
	{
		ImGui::Text("Depth of field:\n");
		ImGui::Text("	Change focal distance: <-K L->\n");
		ImGui::Text("	Focal distance: %.2f\n", focalDistance);
		ImGui::Text("	Change focal range: <-N M->\n");
		ImGui::Text("	Focal range: %.2f\n", focalRange);
	}

	// Render UI
	ImGui::Render();
}

void App1::ControlScene()
{
	// Light controls
	float pointLightSpeed = 5 * timer->getTime();
	if (input->isKeyDown('J'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x - pointLightSpeed, lights[0]->getPosition().y, lights[0]->getPosition().z);
	}
	else if (input->isKeyDown('L'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x + pointLightSpeed, lights[0]->getPosition().y, lights[0]->getPosition().z);
	}
	if (input->isKeyDown('U'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x, lights[0]->getPosition().y - pointLightSpeed, lights[0]->getPosition().z);
	}
	else if (input->isKeyDown('O'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x, lights[0]->getPosition().y + pointLightSpeed, lights[0]->getPosition().z);
	}
	if (input->isKeyDown('K'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x, lights[0]->getPosition().y, lights[0]->getPosition().z - pointLightSpeed);
	}
	else if (input->isKeyDown('I'))
	{
		lights[0]->setPosition(lights[0]->getPosition().x, lights[0]->getPosition().y, lights[0]->getPosition().z + pointLightSpeed);
	}

	// Sphere controls
	float sphereSpeed = 5 * timer->getTime();
	if (input->isKeyDown('F'))
	{
		sphere->SetPosition(sphere->GetPosition().x - sphereSpeed, sphere->GetPosition().y, sphere->GetPosition().z);
	}
	else if (input->isKeyDown('H'))
	{
		sphere->SetPosition(sphere->GetPosition().x + sphereSpeed, sphere->GetPosition().y, sphere->GetPosition().z);
	}
	if (input->isKeyDown('R'))
	{
		sphere->SetPosition(sphere->GetPosition().x, sphere->GetPosition().y - sphereSpeed, sphere->GetPosition().z);
	}
	else if (input->isKeyDown('Y'))
	{
		sphere->SetPosition(sphere->GetPosition().x, sphere->GetPosition().y + sphereSpeed, sphere->GetPosition().z);
	}
	if (input->isKeyDown('G'))
	{
		sphere->SetPosition(sphere->GetPosition().x, sphere->GetPosition().y, sphere->GetPosition().z - sphereSpeed);
	}
	else if (input->isKeyDown('T'))
	{
		sphere->SetPosition(sphere->GetPosition().x, sphere->GetPosition().y, sphere->GetPosition().z + sphereSpeed);
	}

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

	// Toggle wire frame mode
	if (input->isKeyDown('M') && !wasIKeyDownLastFrame)
	{
		renderer->setWireframeMode(!renderer->getWireframeState());
	}
	wasIKeyDownLastFrame = input->isKeyDown('M');
}

void App1::UpdateObjects()
{
	camera->update();
	skybox->SetPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	directionalLight->UpdateMatrices(screenWidth, screenHeight);
	for each (MyLight* light in lights)
	{
		light->UpdateMatrices(screenWidth, screenHeight);
	}
}

