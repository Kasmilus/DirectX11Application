#pragma once
#include "..\DXFramework\BaseShader.h"

using namespace std;
using namespace DirectX;


class GeometryShader : public BaseShader
{

public:
	GeometryShader(ID3D11Device* device, HWND hwnd);
	~GeometryShader();

	struct WindBufferType
	{
		float time;
		float windStrength;
		float windFreq;
		float padding;
	};

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_noise_length, ID3D11ShaderResourceView* texture_noise_wind, float currentTime, float windStrength, float windFreq);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(WCHAR*, WCHAR*);
	void initShader(WCHAR* vsFilename, WCHAR* gsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* windBuffer;
};
