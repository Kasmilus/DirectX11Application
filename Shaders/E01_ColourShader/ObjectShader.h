#pragma once

#include "MyBaseShader.h"
#include "../DXFramework/Light.h"

using namespace std;
using namespace DirectX;


class ObjectShader : public MyBaseShader
{

public:

	struct LightBufferType
	{
		XMFLOAT4 diffuseColour;
		XMFLOAT4 ambientColour;
		XMFLOAT3 lightPosition;
		float padding;
	};
	struct CameraBufferType
	{
		XMFLOAT3 position;
		float padding;
	};

	ObjectShader(ID3D11Device* device, HWND hwnd);
	~ObjectShader();


	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, XMFLOAT3 cameraPosition, Light* light, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
};
