#include "MyLight.h"



MyLight::MyLight(D3D* renderer)
{
	shadowMap = new RenderTexture(renderer->getDevice(), 1024, 1024, SCREEN_NEAR, SCREEN_DEPTH);
	shadowCubemap = new RenderTextureCubemap(renderer->getDevice(), SCREEN_NEAR, SCREEN_DEPTH);

	SetAttenuation();
}


MyLight::~MyLight()
{

}

ID3D11ShaderResourceView * MyLight::GetShadowResourceView()
{
	if (isDirectional)
		return shadowMap->getShaderResourceView();
	else
		return shadowCubemap->getShaderResourceView();
}

void MyLight::UpdateMatrices(float screenWidth, float screenHeight)
{
	if (IsDirectional())
	{

	}
	generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	generateViewMatrix();
	generateOrthoMatrix(screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
}

void MyLight::UpdateShadowMap(D3D* renderer, std::function<void(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)> renderScene)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	if (IsDirectional())
	{
		shadowMap->setRenderTarget(renderer->getDeviceContext());
		shadowMap->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

		// get world view projection matrices from light point position
		worldMatrix = renderer->getWorldMatrix();
		viewMatrix = getViewMatrix();
		projectionMatrix = getProjectionMatrix();

		renderScene(worldMatrix, viewMatrix, projectionMatrix);
	}
	else
	{
		// Needed to Calculate view matrix
		XMVECTOR pos, lookAt, up;
		pos = XMLoadFloat3(&getPosition());
		worldMatrix = renderer->getWorldMatrix();
		projectionMatrix = shadowCubemap->getProjectionMatrix();
		float x = getPosition().x;
		float y = getPosition().y;
		float z = getPosition().z;

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

			shadowCubemap->setRenderTarget(renderer->getDeviceContext(), i);
			shadowCubemap->clearRenderTarget(renderer->getDeviceContext(), i, 0.0f, 0.0f, 0.0f, 1.0f);

			renderScene(worldMatrix, viewMatrix, projectionMatrix);
		}
	}




}
