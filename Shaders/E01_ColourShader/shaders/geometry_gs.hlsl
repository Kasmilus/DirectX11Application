// Gaussian blur shader

Texture2D noiseLength : register(t0);
Texture2D noiseWind : register(t1);
SamplerState sampleType : register(s0);

cbuffer MatrixBuffer : register(cb0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer WindBuffer : register(cb1)
{
	float time;
	float windFreq;
	float windStrength;
	float padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 positionSV : SV_POSITION;
};
// pixel input type
struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

[maxvertexcount(98)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triStream)
{
	OutputType output;

	// Grass strand properties
	float Gravity = 0.05f;
	float Length = 4.5f;
	float Width = 0.4f;	// At base
	// Randomization
	float DirectionIntensity = 0.7f;
	float LengthIntensity = 15;

	// Rename variables for clarity
	float4 P1 = input[0].position;
	float4 P2 = input[1].position;
	float4 P3 = input[2].position;

	float2 T1 = input[0].tex;
	float2 T2 = input[1].tex;
	float2 T3 = input[2].tex;

	float4 N1 = float4(input[0].normal, 0);

	// Strand variables
	float4 P = (P1 + P2 + P3) / 3.0f;	// Get position in between all vertices
	float4 N = N1;	// I'm using a plane so all normals are pointing the same direction
	float2 T = (T1 + T2 + T3) / 3.0f;
	float4 Dir = float4((P2 - P1).xyz, 0.0f);	// Decide on direction
	Dir = normalize(Dir);
   
	float3 lengthDir = noiseLength.SampleLevel(sampleType, T, 0).xyz;
	float3 windDir = noiseWind.SampleLevel(sampleType, T * sin(time * windFreq/10), 0).xyz;
	windDir = normalize(windDir);

	// Randomization
	Length = Length + lengthDir.z * LengthIntensity;
	lengthDir = (lengthDir * 2) - 1;
	lengthDir *= DirectionIntensity;
	// Wind
	windDir = (windDir * 2) - 1;
	windDir *= cos(time * windFreq);
	windDir = clamp(windDir, -windStrength, windStrength);
	N = N + float4(lengthDir + windDir, 0.0f);
	N = normalize(N);

	// Partition the strand so it can bend
	const int PARTS = 8;
	float t0 = 0;
	float t1 = 0;
	for (float i = 0; i < PARTS; ++i)
	{
		// Get position in between bottom and top
		t0 = t1;
		t1 = (i + 1) / PARTS;

		float4 heightVector0 = float4(0, Length * t0, 0, 0) * Gravity * t0;
		float4 p0 = normalize(N - heightVector0) * (Length * t0);
		float4 heightVector1 = float4(0, Length * t1, 0, 0) * Gravity * t1;
		float4 p1 = normalize(N - heightVector1) * (Length * t1);

		float widthBottom = (1 - t0) * Width;
		float widthTop = (1 - t1) * Width;
		float4 w0 = Dir * widthBottom;
		float4 w1 = Dir * widthTop;

		// Define output positions
		float4 OP1 = P + p1 - w1;
		float4 OP2 = P + p0 - w0;
		float4 OP3 = P + p0 + w0;
		float4 OP4 = P + p1 + w1;

		OP1 = mul(OP1, worldMatrix);
		OP1 = mul(OP1, viewMatrix);
		OP1 = mul(OP1, projectionMatrix);
		OP2 = mul(OP2, worldMatrix);
		OP2 = mul(OP2, viewMatrix);
		OP2 = mul(OP2, projectionMatrix);
		OP3 = mul(OP3, worldMatrix);
		OP3 = mul(OP3, viewMatrix);
		OP3 = mul(OP3, projectionMatrix);
		OP4 = mul(OP4, worldMatrix);
		OP4 = mul(OP4, viewMatrix);
		OP4 = mul(OP4, projectionMatrix);

		// Define output tex coords
		float2 OTEX1 = float2(0, t1);
		float2 OTEX2 = float2(0, t0);
		float2 OTEX3 = float2(1, t0);
		float2 OTEX4 = float2(1, t1);

		// Construct 2 triangles(front quad)
		// 1
		output.position = OP1;
		output.tex = OTEX1;
		output.normal = N;
		triStream.Append(output);

		output.position = OP2;
		output.tex = OTEX2;
		output.normal = N;
		triStream.Append(output);

		output.position = OP3;
		output.tex = OTEX3;
		output.normal = N;
		triStream.Append(output);
		triStream.RestartStrip();

		// 2
        if(i != PARTS-1)
        {
        
            output.position = OP1;
            output.tex = OTEX1;
            output.normal = N;
            triStream.Append(output);


            output.position = OP3;
            output.tex = OTEX3;
            output.normal = N;
            triStream.Append(output);

            output.position = OP4;
            output.tex = OTEX4;
            output.normal = N;
            triStream.Append(output);
            triStream.RestartStrip();
        }
		// Construct 2 triangles(back quad)
		// 1
		output.position = OP3;
		output.tex = OTEX3;
		output.normal = N;
		triStream.Append(output);

		output.position = OP2;
		output.tex = OTEX2;
		output.normal = N;
		triStream.Append(output);

		output.position = OP1;
		output.tex = OTEX1;
		output.normal = N;
		triStream.Append(output);
		triStream.RestartStrip();

		// 2
        if (i != PARTS - 1)
        {
            output.position = OP4;
            output.tex = OTEX4;
            output.normal = N;
            triStream.Append(output);

            output.position = OP3;
            output.tex = OTEX3;
            output.normal = N;
            triStream.Append(output);

            output.position = OP1;
            output.tex = OTEX1;
            output.normal = N;
            triStream.Append(output);
            triStream.RestartStrip();
        }
    }
}