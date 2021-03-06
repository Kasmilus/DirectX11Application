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
	D3D11_BUFFER_DESC materialBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerComparisonDesc;

	// Load (+ compile) shader files
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
	shadowMapBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	shadowMapBufferDesc.ByteWidth = sizeof(ShadowMapBufferType);
	shadowMapBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	shadowMapBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	shadowMapBufferDesc.MiscFlags = 0;
	shadowMapBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&shadowMapBufferDesc, NULL, &shadowMapBuffer);

	// --- MATERIAL --- //
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);

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


void ObjectShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, XMFLOAT3 cameraPosition, vector<MyLight*> lights, MyLight* directionalLight, ID3D11ShaderResourceView* texture_base, ID3D11ShaderResourceView* texture_normal, ID3D11ShaderResourceView* texture_metallic, ID3D11ShaderResourceView* texture_roughness, ID3D11ShaderResourceView* texture_envCubemap, XMFLOAT2 shadowMapSize, float dirShadowMapQuality, float pointShadowMapQuality, XMFLOAT3 mCol, float mMetallic, float mRoughness, bool useGGXDistribution, bool useGGXGeometry)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	LightBufferType* lightDataPtr;
	CameraBufferType* cameraDataPtr;
	ShadowMapBufferType* shadowMapDataPtr;
	MaterialBufferType* materialDataPtr;
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
	// Matrix
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = tworld;// worldMatrix;
	matrixDataPtr->view = tview;
	matrixDataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Camera
	result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->position = cameraPosition;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);

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

	// Material
	result = deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	materialDataPtr = (MaterialBufferType*)mappedResource.pData;
	materialDataPtr->materialColour = XMFLOAT4(mCol.x, mCol.y, mCol.z, 1.0f);
	materialDataPtr->materialMetallic = mMetallic;
	materialDataPtr->materialRoughness = mRoughness;
	materialDataPtr->distributionFunction = useGGXDistribution == true ? 0 : 1;
	materialDataPtr->geometryFunction = useGGXGeometry == true ? 0 : 1;
	deviceContext->Unmap(materialBuffer, 0);
	deviceContext->PSSetConstantBuffers(2, 1, &materialBuffer);

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

	ID3D11ShaderResourceView* nullSRV = nullptr;
	for(int i=0; i < 10; ++i)
		deviceContext->PSSetShaderResources(i, 1, &nullSRV);
}


