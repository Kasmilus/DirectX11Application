#pragma once

#include "MyBaseMesh.h"

using namespace DirectX;

class MyTesselationPlane : public MyBaseMesh
{

public:
	MyTesselationPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~MyTesselationPlane();

	void sendData(ID3D11DeviceContext*) override;

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};

