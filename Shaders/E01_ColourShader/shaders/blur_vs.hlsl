// colour vertex shader
// Simple geometry pass
// texture coordinates and normals will be ignored.

cbuffer MatrixBuffer : register(cb0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer ScreenSizeBuffer : register(cb1)
{
	float screenWidth;
	float screenHeight;
	float isHorizontal;	// 0 - false(vertical), 1 - true(horizontal)
	float padding;
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
	float2 tex : TEXCOORD0;
	float2 texCoordLeft1 : TEXCOORD1;
	float2 texCoordLeft2 : TEXCOORD2;
	float2 texCoordLeft3 : TEXCOORD3;
	float2 texCoordRight1 : TEXCOORD4;
	float2 texCoordRight2 : TEXCOORD5;
	float2 texCoordRight3 : TEXCOORD6;
	float2 texCoordUp1 : TEXCOORD7;
	float2 texCoordUp2 : TEXCOORD8;
	float2 texCoordUp3 : TEXCOORD9;
	float2 texCoordDown1 : TEXCOORD10;
	float2 texCoordDown2 : TEXCOORD11;
	float2 texCoordDown3 : TEXCOORD12;
	int isHorizontal : TEXCOORD13;
};

OutputType main(InputType input)
{
	OutputType output;
	float texelWidth, texelHeight;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;

	// Determine the floating point size of a texel for a screen with this specific width.
	texelWidth = 1.0f / screenWidth;
	texelHeight = 1.0f / screenHeight;

	// Create UV coordinates for the pixel and its neighbors
	output.texCoordLeft1 = input.tex + float2(-texelWidth, 0);
	output.texCoordLeft2 = input.tex + float2(-texelWidth * 2, 0);
	output.texCoordLeft3 = input.tex + float2(-texelWidth * 3, 0);
	output.texCoordRight1 = input.tex + float2(texelWidth, 0);
	output.texCoordRight2 = input.tex + float2(texelWidth * 2, 0);
	output.texCoordRight3 = input.tex + float2(texelWidth * 3, 0);
	output.texCoordUp1 = input.tex + float2(0, -texelHeight);
	output.texCoordUp2 = input.tex + float2(0, -texelHeight * 2);
	output.texCoordUp3 = input.tex + float2(0, -texelHeight * 3);
	output.texCoordDown1 = input.tex + float2(0, texelHeight);
	output.texCoordDown2 = input.tex + float2(0, texelHeight * 2);
	output.texCoordDown3 = input.tex + float2(0, texelHeight * 3);

	output.isHorizontal = isHorizontal;

	return output;
}