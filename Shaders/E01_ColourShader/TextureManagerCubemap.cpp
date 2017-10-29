#include "TextureManagerCubemap.h"



TextureManagerCubemap::TextureManagerCubemap(ID3D11Device* ldevice, ID3D11DeviceContext* ldeviceContext)
{
	device = ldevice;
	deviceContext = ldeviceContext;
}


TextureManagerCubemap::~TextureManagerCubemap()
{
	if (texture)
	{
		texture->Release();
		texture = 0;
	}
}

void TextureManagerCubemap::loadCubemapTexture(std::string uid, WCHAR* filename)
{
	HRESULT result;

	// check if file exists
	if (!filename)
	{
		filename = L"../res/DefaultDiffuse.png";
	}
	// if not set default texture
	if (!does_file_exist(filename))
	{
		// change default texture
		filename = L"../res/DefaultDiffuse.png";
	}

	// check file extension for correct loading function.
	std::wstring fn(filename);
	std::string::size_type idx;
	std::wstring extension;

	idx = fn.rfind('.');

	if (idx != std::string::npos)
	{
		extension = fn.substr(idx + 1);
	}
	else
	{
		// No extension found
	}

	// Load the texture in.
	if (extension == L"dds")
	{
		unsigned int bindFlags = D3D11_BIND_SHADER_RESOURCE;
		unsigned int miscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		result = CreateDDSTextureFromFileEx(device, deviceContext, filename, 0, D3D11_USAGE_DEFAULT, bindFlags, (unsigned int)0, miscFlags, true, NULL, &texture, nullptr);
	}
	else
	{
		MessageBox(NULL, L"Texture loading error", L"ERROR", MB_OK);
		return;
	}

	if (FAILED(result))
	{
		MessageBox(NULL, L"Texture loading error", L"ERROR", MB_OK);
	}
	else
	{
		textureMap.insert(std::make_pair(uid, texture));
	}

}
// Return texture as a shader resource.
ID3D11ShaderResourceView* TextureManagerCubemap::getCubemapTexture(std::string uid)
{
	if (textureMap.find(uid) != textureMap.end())
	{
		// texture exists
		return textureMap.at(uid);
	}
	else
	{
		return textureMap.at("default");
	}
}

bool TextureManagerCubemap::does_file_exist(const WCHAR *fname)
{
	std::ifstream infile(fname);
	return infile.good();
}

