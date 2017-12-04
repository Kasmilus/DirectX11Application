// 
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
	windStrength = 0.65f;
	windFreq = 0.7f;
	// Tessellation
	minTessFactor = 1;
	maxTessFactor = 4;
	minTessDist = 50;
	maxTessDist = 5;
	// Depth of field
	focalDistance = 0.77f;
	focalRange = 0.55f;

	// Shadow maps quality
	shadowMapSize = XMFLOAT2(1024, 1024);
	dirShadowMapQuality = 2;
	pointShadowMapQuality = 2;
	// material
	mCol = XMFLOAT3(0, 0, 0);
	mRoughness = 0;
	mMetallic = 0;
	ggxDistribution = true;
	ggxGeometry = true;

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
	textureMgr->loadTexture("black", L"../res/black.png");

	// MESHES
	skyboxMesh = new SkyboxMesh(renderer->getDevice(), renderer->getDeviceContext());
	floorMesh = new MyTesselationPlane(renderer->getDevice(), renderer->getDeviceContext(), 25);
	objectMesh = new MyModelMesh(renderer->getDevice(), renderer->getDeviceContext(), "../res/dwarf.obj");
	sphereMesh = new MyModelMesh(renderer->getDevice(), renderer->getDeviceContext(), "../res/sphere.obj");
	grassGeometryMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 50);
	grassGroundPlane = new QuadMesh(renderer->getDevice(), renderer->getDeviceContext());

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
	directionalLight->SetIsActive(true);

	MyLight* light = new MyLight(renderer);
	light->setDiffuseColour(2.0f, 0.0f, 0.0f, 1.0f);
	light->setPosition(1.5f, -2.0f, -2.2f);
	light->SetRadius(8);
	light->SetAttenuation(1, 0.5f, 0.75f);
	light->SetDirectional(false);
	light->SetIsActive(true);
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
	light4->setDiffuseColour(4.0f, 4.0f, 0.0f, 1.0f);
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

	// Shaders
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 1024, 576, 0, 0);
	colourShader = new ColourShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	blurShader = new BlurShader(renderer->getDevice(), hwnd);
	DOFShader = new DepthOfFieldShader(renderer->getDevice(), hwnd);
	depthTesselationShader = new DepthTesselationShader(renderer->getDevice(), hwnd);

	// Render objects
	// Skybox
	skybox = new RenderObject(skyboxMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	// Floor
	floor = new RenderObject(floorMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	floor->SetPosition(-15, -5, -10);
	floor->SetScale(1.2f, 0.25f, 1.2f);
	// Dwarf
	dwarf = new RenderObject(objectMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	dwarf->SetStaticReflectionsTexture(textureMgr->getTexture("skybox"));
	dwarf->UseDynamicReflections(false);
	// Sphere
	sphere = new RenderObject(sphereMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	sphere->SetPosition(-5, 0, 0);
	sphere->SetStaticReflectionsTexture(textureMgr->getTexture("skybox"));
	sphere->UseDynamicReflections(true);
	// Grass
	grass = new RenderObject(grassGeometryMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	grass->SetPosition(-30.0f, -5.2f, 0);
	grass->SetScale(0.3f, 0.3f, 0.3f);
	grassGround = new RenderObject(grassGeometryMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);
	grassGround->SetPosition(-30.0f, -5.2f, 0);
	grassGround->SetScale(0.3f, 0.3f, 0.3f);
	// Ortho mesh
	orthoMeshObj = new RenderObject(orthoMesh, skyboxShader, objectShader, displacementShader, grassGeometryShader, colourShader, depthShader, depthTesselationShader, blurShader, DOFShader);

	// Set defualt shader params for objects
	UpdateShaderQualityParams();
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
		renderScene(worldMatrix, viewMatrix, projectionMatrix, false);
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

void App1::renderReflectionCubemaps()
{
	// Render reflection cubemap(pass function pointer)
	// Sphere
	if (sphere->IsUsingDynamicReflections())
	{
		skybox->SetPosition(sphere->GetPosition());
		sphere->UpdateReflectionCubemap(renderer, std::bind(&App1::renderScene, this, _1, _2, _3, _4));
	}

	// Dwarf
	if (dwarf->IsUsingDynamicReflections())
	{
		skybox->SetPosition(dwarf->GetPosition());
		dwarf->UpdateReflectionCubemap(renderer, std::bind(&App1::renderScene, this, _1, _2, _3, _4));
	}

	// Move camera back
	skybox->SetPosition(camera->getPosition());

	renderer->setBackBufferRenderTarget();
}

void App1::renderSceneToTexture()
{
	// Render reflection maps
	renderReflectionCubemaps();

	// Render the scene
	sceneTextureCurrent->setRenderTarget(renderer->getDeviceContext());
	sceneTextureCurrent->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	renderScene(worldMatrix, viewMatrix, projectionMatrix, true);

	renderer->setBackBufferRenderTarget();
}
// Render objects without any post-processing effects
void App1::renderScene(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection, bool renderReflection)
{
	// Render dwarf
	dwarf->RenderObjectShader(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("object_base"), textureMgr->getTexture("object_normal"), textureMgr->getTexture("object_metallic"), textureMgr->getTexture("object_roughness"), mCol, mMetallic, mRoughness, ggxDistribution, ggxGeometry, renderReflection);
	// Render sphere
	sphere->RenderObjectShader(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("object_base"), textureMgr->getTexture("object_normal"), textureMgr->getTexture("object_metallic"), textureMgr->getTexture("object_roughness"), mCol, mMetallic, mRoughness, ggxDistribution, ggxGeometry, renderReflection);
	// Render floor
	floor->RenderDisplacement(renderer->getDeviceContext(), world, view, projection, camera->getPosition(), lights, directionalLight, textureMgr->getTexture("brick_base"), textureMgr->getTexture("brick_normal"), textureMgr->getTexture("brick_metallic"), textureMgr->getTexture("brick_roughness"), textureMgr->getTexture("brick_height"), textureMgr->getTexture("skybox"));
	// Render grass
	grass->RenderGrass(renderer->getDeviceContext(), world, view, projection, textureMgr->getTexture("grass_base"), textureMgr->getTexture("grass_noise_length"), textureMgr->getTexture("grass_noise_wind"), currentTime, windStrength, windFreq);
	grassGround->RenderTexture(renderer->getDeviceContext(), world, view, projection, textureMgr->getTexture("black"));
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
	orthoMeshObj->RenderDepthOfField(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, sceneTextureCurrent->getShaderResourceView(), depthTexture->getShaderResourceView(), blurTextureUpSampled->getShaderResourceView(), screenWidth, screenHeight, useDOF, useVignette, useBW);

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
	if (ImGui::Button("Toggle wire frame mode"))
	{
		renderer->setWireframeMode(!renderer->getWireframeState());
	}
	// Controls
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Keyboard controls: \n");
	ImGui::BulletText("	Move camera: WSAD\n");
	ImGui::BulletText("	Move sphere: TGFH\n");
	ImGui::BulletText("	Move point light: IKJL\n");
	// Dynamic reflections
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Dynamic reflections:");
	ImGui::Checkbox("Sphere", &sphere->dynamicReflection);
	ImGui::SameLine();
	ImGui::Checkbox("Dwarf", &dwarf->dynamicReflection);
	// Material properties
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Sphere and dwarf material(used on top of textures):");
	float c[3] = { mCol.x,mCol.y,mCol.z };
	ImGui::ColorEdit3("Colour###cc", c);
	mCol = XMFLOAT3(c[0], c[1], c[2]);
	ImGui::SliderFloat("Roughness", &mRoughness, -1.0f, 1.0f);
	ImGui::SliderFloat("Metallic", &mMetallic, -1.0f, 1.0f);
	ImGui::Text("Cook-Torrance term");
	ImGui::Text("Distribution function:");
	ImGui::SameLine();
	ImGui::Checkbox("GGX", &ggxDistribution);
	ImGui::SameLine();
	bool beckamnnDistribution = !ggxDistribution;
	ImGui::Checkbox("Beckmann", &beckamnnDistribution);
	ggxDistribution = !beckamnnDistribution;
	ImGui::Text("Geometry function:");
	ImGui::SameLine();
	ImGui::Checkbox("GGX###ggx2", &ggxGeometry);
	ImGui::SameLine();
	bool ctGeometry = !ggxGeometry;
	ImGui::Checkbox("Cook-Torrance", &ctGeometry);
	ggxGeometry = !ctGeometry;

	// Tesselation
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Floor tessellation:");
	ImGui::SliderFloat("Min distance", &minTessDist, 20.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Max distance", &maxTessDist, 1.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Min factor", &minTessFactor, 1.0f, 4.0f, "%.1f");
	ImGui::SliderFloat("Max factor", &maxTessFactor, 4.0f, 8.0f, "%.1f");
	// Shadows quality
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Shadow maps:");
	ImGui::Text("Size:");
	if (ImGui::Button("256x256"))
	{
		shadowMapSize = XMFLOAT2(256, 256);
		directionalLight->ChangeShadowMapSize(renderer, shadowMapSize);
	}
	ImGui::SameLine();
	if (ImGui::Button("512x512"))
	{
		shadowMapSize = XMFLOAT2(512, 512);
		directionalLight->ChangeShadowMapSize(renderer, shadowMapSize);
	}
	ImGui::SameLine();
	if (ImGui::Button("1024x1024"))
	{
		shadowMapSize = XMFLOAT2(1024, 1024);

		directionalLight->ChangeShadowMapSize(renderer, shadowMapSize);
	}
	ImGui::Text("Directional light shadows quality:");
	if (ImGui::Button("None"))
		dirShadowMapQuality = 0;
	ImGui::SameLine();
	if (ImGui::Button("Hard"))
		dirShadowMapQuality = 1;
	ImGui::SameLine();
	if (ImGui::Button("Soft"))
		dirShadowMapQuality = 2;
	ImGui::Text("Point light shadows quality:");
	if (ImGui::Button("None##None1"))
		pointShadowMapQuality = 0;
	ImGui::SameLine();
	if (ImGui::Button("Hard##Hard1"))
		pointShadowMapQuality = 1;
	ImGui::SameLine();
	if (ImGui::Button("Soft##Soft1"))
		pointShadowMapQuality = 2;
	// Wind
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Grass wind:");
	ImGui::SliderFloat("Frequency", &windFreq, 0.1f, 1.5f, "%.2f");
	ImGui::SliderFloat("Strength", &windStrength, 0.1f, 1.0f, "%.2f");
	// Post processing
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	if (ImGui::Button("Toggle post-processing effects"))
	{
		postProcessingOn = !postProcessingOn;
	}
	if (postProcessingOn)
	{
		ImGui::Checkbox("Black and White", &useBW);
		ImGui::Checkbox("Vignette", &useVignette);
		ImGui::Checkbox("Depth of field", &useDOF);
		ImGui::SliderFloat("Change focal distance", &focalDistance, -0.99f, 2.0f, "%.2f");
		ImGui::SliderFloat("Change focal range", &focalRange, 0.55f, 6.0f, "%.2f");
	}
	// Lights
	// Dir light
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImVec4 col = ImVec4(directionalLight->getDiffuseColour().x, directionalLight->getDiffuseColour().y, directionalLight->getDiffuseColour().z, 1);
	bool isActive = directionalLight->IsActive();
	ImGui::Text("Directional light");
	ImGui::SameLine();
	ImGui::Checkbox("", &isActive);
	ImGui::ColorEdit3("Colour", (float*)&col);
	directionalLight->setDiffuseColour(col.x, col.y, col.z, 1.0f);
	if(!isActive)
		directionalLight->setDiffuseColour(0, 0, 0, 1.0f);
	directionalLight->SetIsActive(isActive);
	ImGui::NewLine();

	// point lights
	for (int i = 0; i < 4; ++i)
	{
		MyLight* l = lights.at(i);
		float radius = l->GetRadius();
		ImVec4 col = ImVec4(l->getDiffuseColour().x, l->getDiffuseColour().y, l->getDiffuseColour().z, 1);
		bool isActive = l->IsActive();
		float atten[3] = { l->GetAttenuation().x, l->GetAttenuation().y, l->GetAttenuation().z };
		ImGui::PushID(i);
		ImGui::Text("Point light %d", (i + 1));
		ImGui::SameLine();
		ImGui::Checkbox("", &isActive);
		ImGui::ColorEdit3("Colour", (float*)&col);
		ImGui::SliderFloat("Radius", &radius, 4.0f, 15.0f);
		ImGui::InputFloat3("Attenuation", atten);
		ImGui::PopID();
		l->setDiffuseColour(col.x, col.y, col.z, 1.0f);
		l->SetRadius(radius);
		l->SetAttenuation(atten[0], atten[1], atten[2]);
		l->SetIsActive(isActive);
		ImGui::NewLine();

	}
	ImGui::NewLine();
	ImGui::Separator();
	UpdateShaderQualityParams();

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

void App1::UpdateShaderQualityParams()
{
	floor->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	dwarf->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	sphere->SetShadowMapQuality(shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	floor->SetTesselationQuality(minTessFactor, maxTessFactor, minTessDist, maxTessDist);
}

