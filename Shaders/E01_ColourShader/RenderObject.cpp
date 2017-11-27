#include "RenderObject.h"
#include "MyBaseMesh.h"

RenderObject::RenderObject(BaseMesh * mesh, SkyboxShader * skyboxShader, ObjectShader * objectShader, DisplacementShader * displacementShader, GeometryShader * geometryShader, ColourShader * colourShader, DepthShader * depthShader, DepthTesselationShader * depthTesselationShader, BlurShader * blurShader, DepthOfFieldShader * DOFShader)
{
	// Mesh
	this->mesh = mesh;

	// Shaders
	this->skyboxShader = skyboxShader;
	this->objectShader = objectShader;
	this->displacementShader = displacementShader;
	this->geometryShader = geometryShader;
	this->colourShader = colourShader;
	this->depthShader = depthShader;
	this->depthTesselationShader = depthTesselationShader;
	this->blurShader = blurShader;
	this->DOFShader = DOFShader;

	// Default values
	SetPosition(0, 0, 0);
	SetRotation(0, 0, 0);
	SetScale(1, 1, 1);
}

RenderObject::~RenderObject()
{
}

void RenderObject::UpdateReflectionCubemap(D3D * renderer, std::function<void(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)> renderScene)
{
	if (reflectionCubemap == nullptr)
	{
		reflectionCubemap = new RenderTextureCubemap(renderer->getDevice(), SCREEN_NEAR, SCREEN_DEPTH);
	}

	// Needed to Calculate view matrix
	XMVECTOR pos, lookAt, up;
	pos = XMLoadFloat3(&GetPosition());
	worldMatrix = renderer->getWorldMatrix();
	projectionMatrix = reflectionCubemap->getProjectionMatrix();
	float x = GetPosition().x;
	float y = GetPosition().y;
	float z = GetPosition().z;

	// Define directions
	XMFLOAT3 lookAtTargets[6] = {
		XMFLOAT3(x + 1, y, z),
		XMFLOAT3(x - 1, y, z),
		XMFLOAT3(x, y + 1, z),
		XMFLOAT3(x, y - 1, z),
		XMFLOAT3(x, y, z + 1),
		XMFLOAT3(x, y, z - 1)
	};
	XMFLOAT3 upVectors[6] = {
		XMFLOAT3(0, 1, 0),
		XMFLOAT3(0, 1, 0),
		XMFLOAT3(0, 0, -1),
		XMFLOAT3(0, 0, 1),
		XMFLOAT3(0, 1, 0),
		XMFLOAT3(0, 1, 0),
	};
	
	for (int i = 0; i < 6; ++i)
	{
		// Define look and up vectors for that face of the cubemap 
		lookAt = XMVectorSet(lookAtTargets[i].x, lookAtTargets[i].y, lookAtTargets[i].z, 1.0f);
		up = XMVectorSet(upVectors[i].x, upVectors[i].y, upVectors[i].z, 1.0f);

		viewMatrix = XMMatrixLookAtLH(pos, lookAt, up);

		reflectionCubemap->setRenderTarget(renderer->getDeviceContext(), i);
		reflectionCubemap->clearRenderTarget(renderer->getDeviceContext(), i, 0.0f, 0.0f, 1.0f, 1.0f);

		renderScene(worldMatrix, viewMatrix, projectionMatrix);
	}
}

void RenderObject::RenderSkybox(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture)
{
	SetWorldMatrix(world);

	mesh->sendData(deviceContext);
	skyboxShader->setShaderParameters(deviceContext, world, view, projection, texture);
	skyboxShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::RenderObjectShader(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView * texture_base, ID3D11ShaderResourceView * texture_normal, ID3D11ShaderResourceView * texture_metallic, ID3D11ShaderResourceView * texture_roughness, ID3D11ShaderResourceView* texture_envCubemap)
{
	if (texture_envCubemap == nullptr)
	{
		if (reflectionCubemap)
		{
			texture_envCubemap = reflectionCubemap->getShaderResourceView();
		}
		else
		{
			return;	// didn't pass reflection map to the shader and it wasn't generated yet - don't render(should print out some error, add later)
		}
	}

	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;	// yeah, that's not safe but it's just so I don't have to edit framework class

	myMesh->sendData(deviceContext);
	objectShader->setShaderParameters(deviceContext, world, view, projection, cameraPosition, lights, directionalLight, texture_base, texture_normal, texture_metallic, texture_roughness, texture_envCubemap, shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	objectShader->render(deviceContext, myMesh->getIndexCount());
}

void RenderObject::RenderDisplacement(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView * texture_base, ID3D11ShaderResourceView * texture_normal, ID3D11ShaderResourceView * texture_metallic, ID3D11ShaderResourceView * texture_roughness, ID3D11ShaderResourceView * texture_displacement, ID3D11ShaderResourceView* texture_envCubemap)
{
	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;
	myMesh->sendData(deviceContext);
	displacementShader->setShaderParameters(deviceContext, world, view, projection, cameraPosition, lights, directionalLight, texture_base, texture_normal, texture_metallic, texture_roughness, texture_displacement, texture_envCubemap, minTessFactor, maxTessFactor, minTessDist, maxTessDist, shadowMapSize, dirShadowMapQuality, pointShadowMapQuality);
	displacementShader->render(deviceContext, myMesh->getIndexCount());
}

void RenderObject::RenderGrass(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture_base, ID3D11ShaderResourceView * texture_noise_length, ID3D11ShaderResourceView * texture_noise_wind, float currentTime, float windStrength, float windFreq)
{
	SetWorldMatrix(world);

	mesh->sendData(deviceContext);
	geometryShader->setShaderParameters(deviceContext, world, view, projection, texture_base, texture_noise_length, texture_noise_wind, currentTime, windStrength, windFreq);
	geometryShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::RenderTexture(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture)
{
	SetWorldMatrix(world);

	mesh->sendData(deviceContext);
	colourShader->setShaderParameters(deviceContext, world, view, projection, texture);
	colourShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::RenderDepth(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, float focalDistance, float focalRange)
{
	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;
	myMesh->sendData(deviceContext);
	depthShader->setShaderParameters(deviceContext, world, view, projection, focalDistance, focalRange);
	depthShader->render(deviceContext, myMesh->getIndexCount());
}

void RenderObject::RenderTesselationDepth(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, float focalDistance, float focalRange, ID3D11ShaderResourceView * texture_displacement, XMFLOAT3 cameraPosition)
{
	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;
	myMesh->sendData(deviceContext);
	depthTesselationShader->setShaderParameters(deviceContext, world, view, projection, focalDistance, focalRange, texture_displacement, cameraPosition, minTessFactor, maxTessFactor, minTessDist, maxTessDist);
	depthTesselationShader->render(deviceContext, myMesh->getIndexCount());
}

void RenderObject::RenderBlur(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * sceneTextureFirstPass, ID3D11ShaderResourceView * sceneTextureSecondPass, ID3D11ShaderResourceView * depthTexture, float screenWidth, float screenHeight)
{

	mesh->sendData(deviceContext);
	blurShader->setShaderParameters(deviceContext, world, view, projection, sceneTextureFirstPass, depthTexture, screenWidth, screenHeight, true);
	blurShader->render(deviceContext, mesh->getIndexCount());
	mesh->sendData(deviceContext);
	blurShader->setShaderParameters(deviceContext, world, view, projection, sceneTextureSecondPass, depthTexture, screenWidth, screenHeight, false);
	blurShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::RenderDepthOfField(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * sceneTexture, ID3D11ShaderResourceView * depthTexture, ID3D11ShaderResourceView * blurTexture, float screenResX, float screenResY)
{
	SetWorldMatrix(world);

	mesh->sendData(deviceContext);
	DOFShader->setShaderParameters(deviceContext, world, view, projection, sceneTexture, depthTexture, blurTexture, screenResX, screenResY);
	DOFShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::SetWorldMatrix(XMMATRIX& world)
{
	world = XMMatrixMultiply(XMMatrixScaling(scale.x, scale.y, scale.z), XMMatrixTranslation(position.x, position.y, position.z));
}


