// Objject vertex shader

cbuffer MatrixBuffer : register(cb0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(cb1)
{
	float3 cameraPosition;
	float padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 viewDirection : TEXCOORD1;
	float4 lightViewPosition : TEXCOORD2;	// THIS IS NOT SET YET
	float3 lightPos : TEXCOORD3;	// SAME
};

OutputType main(InputType input)
{
	OutputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.viewDirection = normalize(cameraPosition - output.position);	// Set view direction
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix then normalize
	output.normal = mul(input.normal, (float3x3)worldMatrix);	// normal
	output.normal = normalize(output.normal);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);	// Tangent
	output.tangent = normalize(output.tangent);
	output.binormal = mul(input.binormal, (float3x3)worldMatrix);	// Binormal
	output.binormal = normalize(output.binormal);

	return output;
}