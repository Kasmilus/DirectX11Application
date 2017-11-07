#include "DepthTesselationShader.h"


DepthTesselationShader::DepthTesselationShader(ID3D11Device* device, HWND hwnd) : MyBaseShader(device, hwnd)
{
	initShader(L"displacement_vs.cso", L"displacement_hs.cso", L"depthTessellation_ds.cso", L"depthTessellation_ps.cso");
}


DepthTesselationShader::~DepthTesselationShader()
{
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
	if (DepthOfFieldBuffer)
	{
		DepthOfFieldBuffer->Release();
		DepthOfFieldBuffer = 0;
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


void DepthTesselationShader::initShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC depthOfFieldBufferDesc;
	D3D11_BUFFER_DESC tessellationBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

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
	// --- DEPTH OF FIELD --- //
	depthOfFieldBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	depthOfFieldBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	depthOfFieldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	depthOfFieldBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	depthOfFieldBufferDesc.MiscFlags = 0;
	depthOfFieldBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&depthOfFieldBufferDesc, NULL, &DepthOfFieldBuffer);
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

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
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
}

void DepthTesselationShader::initShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}

void DepthTesselationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, float focalDistance, float focalRange, ID3D11ShaderResourceView* texture_displacement, XMFLOAT3 cameraPosition)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	CameraBufferType* cameraDataPtr;
	TessellationBufferType* tessellationPtr;
	DepthOfFieldBufferType* dataPtrDOF;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Matrix
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = tworld;
	matrixDataPtr->view = tview;
	matrixDataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	// Tessellation
	result = deviceContext->Map(tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessellationPtr = (TessellationBufferType*)mappedResource.pData;
	tessellationPtr->minTesselationDistance = 30;
	tessellationPtr->maxTesselationDistance = 5;
	tessellationPtr->minTesselationFactor = 1;
	tessellationPtr->maxTesselationFactor = 8;
	deviceContext->Unmap(tessellationBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &tessellationBuffer);
	// Camera position
	result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->position = cameraPosition;
	cameraDataPtr->padding = 0;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(2, 1, &cameraBuffer);
	// --- DOMAIN SHADER --- //
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);	// Matrix
	deviceContext->DSSetConstantBuffers(1, 1, &cameraBuffer);	// Camera
	deviceContext->DSSetShaderResources(0, 1, &texture_displacement);	// Displacement texture
	// DOF
	result = deviceContext->Map(DepthOfFieldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtrDOF = (DepthOfFieldBufferType*)mappedResource.pData;
	dataPtrDOF->focalDistance = focalDistance;
	dataPtrDOF->focalRange = focalRange;
	deviceContext->Unmap(DepthOfFieldBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &DepthOfFieldBuffer);

}