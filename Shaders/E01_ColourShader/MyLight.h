#pragma once

#include "../DXFramework/DXF.h"
#include "RenderTextureCubemap.h"
#include <functional>


class MyLight :
	public Light
{
public:
	MyLight(D3D* renderer);
	~MyLight();

	inline void SetDirectional(bool newDirectional) { isDirectional = newDirectional; }
	inline bool IsDirectional() { return isDirectional; }
	inline void SetIsActive(bool newIsActive) { isActive = newIsActive; }
	inline bool IsActive() { return isActive; }
	inline void SetAttenuation(float constantFactor = 1.0f, float linearFactor = 0.125f, float quadraticFactor = 0.01f) {
		attenuation.x = constantFactor;
		attenuation.y = linearFactor;
		attenuation.z = quadraticFactor;
	}
	inline XMFLOAT3 GetAttenuation() { return attenuation; }
	inline void SetRadius(float newRadius) {radius = newRadius; }
	inline float GetRadius() { return radius; }
	ID3D11ShaderResourceView* GetShadowResourceView();

	void UpdateMatrices(float screenWidth, float screenHeight);
	void UpdateShadowMap(D3D* renderer, std::function<void(XMMATRIX &world, XMMATRIX &view, XMMATRIX &projection)> renderScene);

private:
	bool isDirectional;
	bool isActive;

	XMFLOAT3 attenuation;	// x - constant factor, y - linear, z - quadratic
	float radius;

	RenderTexture* shadowMap;
	RenderTextureCubemap* shadowCubemap;
};

