#include "ObjectShader.h"


ObjectShader::ObjectShader(ID3D11Device* device, HWND hwnd) : MyBaseShader(device, hwnd)
{
	initShader(L"object_vs.cso", L"object_ps.cso");
}


ObjectShader::~ObjectShader()
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
	MyBaseShader::~MyBaseShader();
}


void ObjectShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC shadowMapBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerComparisonDesc;

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

	// Camera buffer description
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Light buffer description
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Shadow map buffer description
	shadowMapBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	shadowMapBufferDesc.ByteWidth = sizeof(ShadowMapBufferType);
	shadowMapBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	shadowMapBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	shadowMapBufferDesc.MiscFlags = 0;
	shadowMapBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&shadowMapBufferDesc, NULL, &shadowMapBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

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


void ObjectShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_envCubemap, XMFLOAT2 shadowMapSize, float dirShadowMapQuality, float pointShadowMapQuality)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	LightBufferType* lightDataPtr;
	CameraBufferType* cameraDataPtr;
	ShadowMapBufferType* shadowMapDataPtr;
	unsigned int bufferNumber;
	XMMATRIX tworld, tview, tproj, tLightViewMatrix, tLightProjectionMatrix;
	ID3D11ShaderResourceView* shadowMaps[4] = { nullptr, nullptr, nullptr, nullptr };
	ID3D11ShaderResourceView* directionalShadowMap = directionalLight->GetShadowResourceView();

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	tLightViewMatrix = XMMatrixTranspose(directionalLight->getViewMatrix());
	tLightProjectionMatrix = XMMatrixTranspose(directionalLight->getProjectionMatrix());

	// ----- VERTEX SHADER ----- //

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixDataPtr->world = tworld;// worldMatrix;
	matrixDataPtr->view = tview;
	matrixDataPtr->projection = tproj;

	// Unlock the constant buffer.
	deviceContext->Unmap(matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	// Camera
	result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->position = cameraPosition;
	deviceContext->Unmap(cameraBuffer, 0);
	bufferNumber++;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &cameraBuffer);

	// ----- PIXEL SHADER ----- //

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
		if (lights[i]->IsActive())
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
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Shadow map quality
	result = deviceContext->Map(shadowMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	shadowMapDataPtr = (ShadowMapBufferType*)mappedResource.pData;
	shadowMapDataPtr->shadowMapSize = shadowMapSize;
	shadowMapDataPtr->directionalShadowMapQuality = dirShadowMapQuality;
	shadowMapDataPtr->pointShadowMapQuality = pointShadowMapQuality;
	deviceContext->Unmap(shadowMapBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &shadowMapBuffer);

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

void ObjectShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateClampPoint);
	deviceContext->PSSetSamplers(2, 1, &sampleStateComparison);

	// Base render function.
	MyBaseShader::render(deviceContext, indexCount);
}


