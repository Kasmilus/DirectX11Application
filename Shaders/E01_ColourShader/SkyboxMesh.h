#pragma once

#include "../DXFramework/BaseMesh.h"

using namespace DirectX;

class SkyboxMesh : public BaseMesh
{

public:
	SkyboxMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 20);
	~SkyboxMesh();

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};
