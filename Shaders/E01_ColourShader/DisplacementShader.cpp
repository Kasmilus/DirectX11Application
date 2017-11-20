// tessellation shader.cpp
#include "DisplacementShader.h"


DisplacementShader::DisplacementShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"displacement_vs.cso", L"displacement_hs.cso", L"displacement_ds.cso", L"object_ps.cso");
}


DisplacementShader::~DisplacementShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (tessellationBuffer)
	{
		tessellationBuffer->Release();
		tessellationBuffer = 0;
	}
	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
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

void DisplacementShader::initShader(WCHAR* vsFilename,  WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerComparisonDesc;

	// Load shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// --- MATRIX --- //
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	// --- TESSELATION --- //
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&tessellationBufferDesc, NULL, &tessellationBuffer);
	// --- CAMERA --- //
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);
	// --- LIGHT --- //
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Required a CLAMPED sampler for sampling the depth map
	//samplerDesc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleStateClampPoint);

	// Required a CLAMPED sampler for sampling the depth map
	samplerComparisonDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT; // D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT
	samplerComparisonDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerComparisonDesc, &sampleStateComparison);

}

void DisplacementShader::initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void DisplacementShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, XMFLOAT3 cameraPosition, Light* light, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_shadow)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	CameraBufferType* cameraDataPtr;
	TessellationBufferType* tessellationPtr;
	LightBufferType* lightDataPtr;
	XMMATRIX tworld, tview, tproj, tLightViewMatrix, tLightProjectionMatrix;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	tLightViewMatrix = XMMatrixTranspose(light->getViewMatrix());
	tLightProjectionMatrix = XMMatrixTranspose(light->getProjectionMatrix());
	// matrix
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = tworld;// worldMatrix;
	matrixDataPtr->view = tview;
	matrixDataPtr->projection = tproj;
	matrixDataPtr->lightView = tLightViewMatrix;
	matrixDataPtr->lightProjection = tLightProjectionMatrix;
	deviceContext->Unmap(matrixBuffer, 0);
	// Tessellation
	result = deviceContext->Map(tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessellationPtr = (TessellationBufferType*)mappedResource.pData;
	tessellationPtr->minTesselationDistance = 30;
	tessellationPtr->maxTesselationDistance = 5;
	tessellationPtr->minTesselationFactor = 1;
	tessellationPtr->maxTesselationFactor = 8;
	deviceContext->Unmap(tessellationBuffer, 0);
	// Camera position
	result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->position = cameraPosition;
	cameraDataPtr->padding = 0;
	deviceContext->Unmap(cameraBuffer, 0);
	// Light
	result = deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightDataPtr = (LightBufferType*)mappedResource.pData;
	lightDataPtr->ambientColour = light->getAmbientColour();
	lightDataPtr->diffuseColour = light->getDiffuseColour();
	lightDataPtr->lightPosition = light->getPosition();
	lightDataPtr->padding = 0;
	deviceContext->Unmap(lightBuffer, 0);

	// --- VERTEX SHADER --- //
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &tessellationBuffer);
	deviceContext->VSSetConstantBuffers(2, 1, &cameraBuffer);


	// --- DOMAIN SHADER --- //
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);	// Matrix
	deviceContext->DSSetConstantBuffers(1, 1, &cameraBuffer);	// Camera
	deviceContext->DSSetConstantBuffers(2, 1, &lightBuffer);	// Light

	deviceContext->DSSetShaderResources(0, 1, &texture_displacement);	// Displacement texture

	// --- PIXEL SHADER --- //
	
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);


	// Set shader texture resource in the pixel shader.
	ID3D11ShaderResourceView* textureArray[5] = { texture_base, texture_normal, texture_metallic, texture_roughness, texture_shadow };
	deviceContext->PSSetShaderResources(0, 5, textureArray);
}

void DisplacementShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateClampPoint);
	deviceContext->PSSetSamplers(2, 1, &sampleStateComparison);

	// Base render function.
	BaseShader::render(deviceContext, indexCount);
}



