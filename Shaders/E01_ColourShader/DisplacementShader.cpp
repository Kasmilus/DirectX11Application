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

void DisplacementShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC shadowMapBufferDesc;
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
	// --- SHADOW MAP --- //
	renderer->CreateBuffer(&lightBufferDesc, NULL, &shadowMapBuffer);
	shadowMapBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	shadowMapBufferDesc.ByteWidth = sizeof(ShadowMapBufferType);
	shadowMapBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	shadowMapBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	shadowMapBufferDesc.MiscFlags = 0;
	shadowMapBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&shadowMapBufferDesc, NULL, &shadowMapBuffer);

	// --- SAMPLERS --- //
	// Regular
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
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// CLAMPED sampler for sampling the depth map
	//samplerDesc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	renderer->CreateSamplerState(&samplerDesc, &sampleStateClampPoint);

	// Comparison sampler
	samplerComparisonDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	samplerComparisonDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
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


void DisplacementShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_displacement, ID3D11ShaderResourceView* texture_envCubemap, float minTessFactor, float maxTessFactor, float minTessDist, float maxTessDist, XMFLOAT2 shadowMapSize, float dirShadowMapQuality, float pointShadowMapQuality)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	CameraBufferType* cameraDataPtr;
	TessellationBufferType* tessellationPtr;
	LightBufferType* lightDataPtr;
	ShadowMapBufferType* shadowMapDataPtr;
	XMMATRIX tworld, tview, tproj, tLightViewMatrix, tLightProjectionMatrix;
	ID3D11ShaderResourceView* shadowMaps[4] = { nullptr, nullptr, nullptr, nullptr };
	ID3D11ShaderResourceView* directionalShadowMap = directionalLight->GetShadowResourceView();

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	tLightViewMatrix = XMMatrixTranspose(directionalLight->getViewMatrix());
	tLightProjectionMatrix = XMMatrixTranspose(directionalLight->getProjectionMatrix());
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
	tessellationPtr->minTesselationDistance = minTessDist;
	tessellationPtr->maxTesselationDistance = maxTessDist;
	tessellationPtr->minTesselationFactor = minTessFactor;
	tessellationPtr->maxTesselationFactor = maxTessFactor;
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
	// Directional
	lightDataPtr->lightAmbientColour = directionalLight->getAmbientColour();
	lightDataPtr->lightDiffuseColour = directionalLight->getDiffuseColour();
	lightDataPtr->lightDirection = directionalLight->getDirection();
	lightDataPtr->padding = 0;
	// Point lights
	int numberOfLights = lights.size() > 4 ? 4 : lights.size();	// Use number of lights provided or only first 4 if there's more than that
	for (int i = 0; i < numberOfLights; ++i)
	{
		// Light
		if(lights[i]->IsActive())
			lightDataPtr->pointLight[i].isActive = 1;
		else
			lightDataPtr->pointLight[i].isActive = 0;
		XMFLOAT3 pos = lights[i]->getPosition();
		lightDataPtr->pointLight[i].lightPositionAndRadius = XMFLOAT4(pos.x, pos.y, pos.z, lights[i]->GetRadius());
		lightDataPtr->pointLight[i].lightAmbientColour = lights[i]->getAmbientColour();
		lightDataPtr->pointLight[i].lightDiffuseColour = lights[i]->getDiffuseColour();
		lightDataPtr->pointLight[i].lightAttenuation = lights[i]->GetAttenuation();

		// Shadow map
		shadowMaps[i] = lights[i]->GetShadowResourceView();
	}
	deviceContext->Unmap(lightBuffer, 0);

	// Shadow map quality
	result = deviceContext->Map(shadowMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	shadowMapDataPtr = (ShadowMapBufferType*)mappedResource.pData;
	shadowMapDataPtr->shadowMapSize = shadowMapSize;
	shadowMapDataPtr->directionalShadowMapQuality = dirShadowMapQuality;
	shadowMapDataPtr->pointShadowMapQuality = pointShadowMapQuality;
	deviceContext->Unmap(shadowMapBuffer, 0);

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
	deviceContext->PSSetConstantBuffers(1, 1, &shadowMapBuffer);


	// Set shader texture resource in the pixel shader.
	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture_base);
	deviceContext->PSSetShaderResources(1, 1, &texture_normal);
	deviceContext->PSSetShaderResources(2, 1, &texture_metallic);
	deviceContext->PSSetShaderResources(3, 1, &texture_roughness);
	deviceContext->PSSetShaderResources(4, 1, &texture_envCubemap);
	deviceContext->PSSetShaderResources(5, 1, &directionalShadowMap);
	if (shadowMaps[0])
		deviceContext->PSSetShaderResources(6, 1, &shadowMaps[0]);
	if (shadowMaps[1])
		deviceContext->PSSetShaderResources(7, 1, &shadowMaps[1]);
	if (shadowMaps[2])
		deviceContext->PSSetShaderResources(8, 1, &shadowMaps[2]);
	if (shadowMaps[3])
		deviceContext->PSSetShaderResources(9, 1, &shadowMaps[3]);
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



