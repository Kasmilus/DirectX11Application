#include "GeometryShader.h"



GeometryShader::GeometryShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"geometry_vs.cso", L"geometry_gs.cso", L"geometry_ps.cso");
}


GeometryShader::~GeometryShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void GeometryShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC windBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	windBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	windBufferDesc.ByteWidth = sizeof(WindBufferType);
	windBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	windBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	windBufferDesc.MiscFlags = 0;
	windBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&windBufferDesc, NULL, &windBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}

void GeometryShader::initShader(WCHAR* vsFilename, WCHAR* gsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadGeometryShader(gsFilename);
}


void GeometryShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_noise_length, ID3D11ShaderResourceView* texture_noise_wind, float currentTime, float windStrength, float windFreq)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	WindBufferType* windDataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);


	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = tworld;// worldMatrix;
	matrixDataPtr->view = tview;
	matrixDataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);

	result = deviceContext->Map(windBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	windDataPtr = (WindBufferType*)mappedResource.pData;
	windDataPtr->time = currentTime;
	windDataPtr->windFreq = windFreq;
	windDataPtr->windStrength = windStrength;
	deviceContext->Unmap(windBuffer, 0);
	deviceContext->GSSetConstantBuffers(1, 1, &windBuffer);

	deviceContext->GSSetShaderResources(0, 1, &texture_noise_length);
	deviceContext->GSSetShaderResources(1, 1, &texture_noise_wind);

	deviceContext->PSSetShaderResources(0, 1, &texture_base);
}

void GeometryShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the sampler state in the pixel shader.
	deviceContext->GSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(0, 1, &sampleState);

	// Base render function.
	BaseShader::render(deviceContext, indexCount);
}