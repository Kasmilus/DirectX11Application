// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

Texture2D displacementTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer MatrixBuffer : register(cb0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

cbuffer CameraBuffer : register(cb1)
{
	float3 cameraPosition;
	float padding;
};

cbuffer LightBufferType : register(cb2)
{
	float4 diffuseColour;
	float4 ambientColour;
	float3 lightPosition;
	float padding2;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 pos : TEXCOORD1;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 viewDirection : TEXCOORD1;
	float4 lightViewPosition : TEXCOORD2;
	float3 lightPos : TEXCOORD3;
};

// Function declarations
float SampleDisplacementMap(float2 uv, in float3 vertexPosition);
void CalculateNormals(inout OutputType output, in float3 vertexPosition);
float3 Sobel(float2 texCoords, in float3 vertexPosition);

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine variables for new vertices
	vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
	output.tex = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
	output.normal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
	output.tangent = uvwCoord.x * patch[0].tangent + uvwCoord.y * patch[1].tangent + uvwCoord.z * patch[2].tangent;
	output.binormal = uvwCoord.x * patch[0].binormal + uvwCoord.y * patch[1].binormal + uvwCoord.z * patch[2].binormal;

	output.normal = normalize(output.normal);
	output.tangent = normalize(output.tangent);
	output.binormal = normalize(output.binormal);

	// --- DISPLACEMENT MAPPING --- //
	float height = SampleDisplacementMap(output.tex, vertexPosition);
	// Offset vertex along the normal
	vertexPosition += float4((height - 1.0f) * output.normal, 0.0f);


	// Calculate the position of the new vertex against the world, view, and projection matrices.
	float4 worldPosition = mul(float4(vertexPosition, 1.0f), worldMatrix);
	output.position = worldPosition;
	output.viewDirection = normalize(cameraPosition - output.position);	// Set view direction
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Calculate vertex position from light point of view
	output.lightViewPosition = worldPosition;
	output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
	output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

	output.lightPos = normalize(lightPosition.xyz - worldPosition.xyz);
	
	CalculateNormals(output, vertexPosition);

    return output;
}

float SampleDisplacementMap(float2 uv, in float3 vertexPosition)
{
	// Choose mipmap level
	const float mipInterval = 20.0f;
	float dist = distance(vertexPosition, cameraPosition);
	float mipLevel = clamp((dist - mipInterval) / mipInterval, 0.0f, 6.0f);

	// Choose scale
	const float SCALE = 4.0f;

	// Sample displacement map
	return displacementTexture.SampleLevel(SampleType, uv, mipLevel).x * SCALE;
}

void CalculateNormals(inout OutputType output, in float3 vertexPosition)
{
	// Calculate new normal based on displacement map using Sobel filter
	float3 normal = Sobel(output.tex, vertexPosition);
	// How much did normal changed?
	float3 normalDelta = output.normal - normal;
	// Set new normal, binormal and tangent vectors
	//output.normal = normal;
	//output.binormal = output.binormal + output.binormal * normalDelta;
	//output.tangent = output.tangent + output.tangent * normalDelta;
}

float3 Sobel(float2 texCoords, in float3 vertexPosition)
{
	// 2048x2048 - hardcoded values now, remember to properly pass texture dimensions later on
	float2 displcementMapDimensions = float2(2048, 2048);

	// Texel size
	float2 txSz = float2(1.0f / displcementMapDimensions.x, 1.0f / displcementMapDimensions.y);

	// Offsets
	float2 o00 = texCoords + float2(-txSz.x, -txSz.y);
	float2 o10 = texCoords + float2(0.0f, -txSz.y);
	float2 o20 = texCoords + float2(txSz.x, -txSz.y);

	float2 o01 = texCoords + float2(-txSz.x, 0.0f);
	float2 o21 = texCoords + float2(txSz.x, 0.0f);

	float2 o02 = texCoords + float2(-txSz.x, txSz.y);
	float2 o12 = texCoords + float2(0, txSz.y);
	float2 o22 = texCoords + float2(txSz.x, txSz.y);

	// Get height of surrounding texels
	float h00 = SampleDisplacementMap(o00, vertexPosition);
	float h10 = SampleDisplacementMap(o10, vertexPosition);
	float h20 = SampleDisplacementMap(o20, vertexPosition);

	float h01 = SampleDisplacementMap(o01, vertexPosition);
	float h21 = SampleDisplacementMap(o21, vertexPosition);

	float h02 = SampleDisplacementMap(o02, vertexPosition);
	float h12 = SampleDisplacementMap(o12, vertexPosition);
	float h22 = SampleDisplacementMap(o22, vertexPosition);

	// Evaluate Sobel filters
	float Gx = h00 - h20 + 2.0f * h01 - 2.0f * h21 + h02 - h22;
	float Gy = h00 + 2.0f * h10 + h20 - h02 - 2.0f * h12 - h22;

	// Z
	float Gz = 0.01f * sqrt(max(0.0f, 1.0f - Gx * Gx - Gy * Gy));

	return normalize(float3(2.0f * Gx, Gz, 2.0f * Gy));
}