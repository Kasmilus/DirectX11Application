// skybox vertex shader



cbuffer MatrixBuffer : register(cb0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
};

OutputType main(InputType input)
{
	OutputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;


	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	// z divided by w will always give 1 so skybox will be rendered behind everything
	// At least in theory, output.position.xyww is not rendering skybox at all(???) so instead I'm subtracting very small number from z component
	output.position = float4(output.position.x, output.position.y, output.position.w-0.000001f, output.position.w);	
	
	// Store the texture coordinates for the pixel shader.
	output.tex = input.position.xyz;

	return output;
}