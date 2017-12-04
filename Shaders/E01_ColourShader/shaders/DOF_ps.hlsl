/*
	Depth of field blur - interpolate between scene texture without blur and the one with blur depending on values stored in DOF depth map
I didn't change name of the shader but it's actually doing all post processing effects(DOF, desaturate, vignette)
*/
Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
Texture2D blurTexture : register(t2);
SamplerState SampleType : register(s0);

cbuffer ScreenSizeBuffer : register(cb0)
{
    float2 screenResolution;
    float effects; // Types of effects to use: 0 - Vignette, 1 - BW, 2 - both
    float UseDOF; // Depth of field 1 - on, 0 - off
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 screenPos : TEXCOORD1;
};

float4 main(InputType input) : SV_TARGET
{
    float4 finalColour;
    float4 sceneColour;
    float4 blurColour;
    float depth;

    // --- DEPTH OF FIELD --- //
	// Get pixel colour in all textures
    sceneColour = sceneTexture.Sample(SampleType, input.tex);
    blurColour = blurTexture.Sample(SampleType, input.tex);
    depth = depthTexture.Sample(SampleType, input.tex).y;
	// Calculate final blur
    if (UseDOF != 0)
        finalColour = lerp(sceneColour, blurColour, depth);
    else
        finalColour = sceneColour;

	// --- VIGNETE --- //
    float vignetteRatio = 1.0f;
    if(effects == 0 || effects == 2)
    {
        // Basic
        float2 pos = (input.position.xy / screenResolution) - float2(0.5f, 0.5f);
        float len = length(pos);
	// Circle inside shouldn't be affected
        const float RADIUS = 0.9f;
        const float INNER_RADIUS = 0.5f;
        vignetteRatio = smoothstep(RADIUS, INNER_RADIUS, len);
    }
	// --- B/W --- //
    if(effects == 1 || effects == 2)
    {
        finalColour = (finalColour.x + finalColour.y + finalColour.z) / 3;
    }

	// Add vignette to the blurred image
    finalColour = finalColour * vignetteRatio;

	// Set the alpha channel to one.
    finalColour.a = 1.0f;

    return finalColour;
}