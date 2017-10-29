#pragma once
/*
	Basically the same thing as TextureManager but with different arguments passed to texture loading function

	I had to copy everything instead of using it due to inaccessibility in parent class
*/

#include <d3d11.h>
#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "../DirectXTK/Inc/WICTextureLoader.h"
#include <string>
#include <fstream>
#include <vector>
#include <map>

using namespace DirectX;

class TextureManagerCubemap
{
public:
	TextureManagerCubemap(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~TextureManagerCubemap();

	void loadCubemapTexture(std::string uid, WCHAR* filename);

	ID3D11ShaderResourceView* getCubemapTexture(std::string uid);

private:
	bool does_file_exist(const WCHAR *fileName);

	ID3D11ShaderResourceView* texture;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	std::map<std::string, ID3D11ShaderResourceView*> textureMap;
};

