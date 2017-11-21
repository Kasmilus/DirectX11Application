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

void RenderObject::RenderSkybox(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, ID3D11ShaderResourceView * texture)
{
	SetWorldMatrix(world);

	mesh->sendData(deviceContext);
	skyboxShader->setShaderParameters(deviceContext, world, view, projection, texture);
	skyboxShader->render(deviceContext, mesh->getIndexCount());
}

void RenderObject::RenderObjectShader(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, XMFLOAT3 cameraPosition, Light * light, ID3D11ShaderResourceView * texture_base, ID3D11ShaderResourceView * texture_normal, ID3D11ShaderResourceView * texture_metallic, ID3D11ShaderResourceView * texture_roughness, ID3D11ShaderResourceView* texture_envCubemap)
{
	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;	// yeah, that's not safe but it's just so I don't have to edit framework class

	myMesh->sendData(deviceContext);
	objectShader->setShaderParameters(deviceContext, world, view, projection, cameraPosition, light, texture_base, texture_normal, texture_metallic, texture_roughness, texture_envCubemap);
	objectShader->render(deviceContext, myMesh->getIndexCount());
}

void RenderObject::RenderDisplacement(ID3D11DeviceContext * deviceContext, XMMATRIX & world, const XMMATRIX & view, const XMMATRIX & projection, XMFLOAT3 cameraPosition, Light * light, ID3D11ShaderResourceView * texture_base, ID3D11ShaderResourceView * texture_normal, ID3D11ShaderResourceView * texture_metallic, ID3D11ShaderResourceView * texture_roughness, ID3D11ShaderResourceView * texture_displacement, ID3D11ShaderResourceView * texture_shadow)
{
	SetWorldMatrix(world);

	MyBaseMesh* myMesh = (MyBaseMesh*)mesh;
	myMesh->sendData(deviceContext);
	displacementShader->setShaderParameters(deviceContext, world, view, projection, cameraPosition, light, texture_base, texture_normal, texture_metallic, texture_roughness, texture_displacement, texture_shadow);
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
	depthTesselationShader->setShaderParameters(deviceContext, world, view, projection, focalDistance, focalRange, texture_displacement, cameraPosition);
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


