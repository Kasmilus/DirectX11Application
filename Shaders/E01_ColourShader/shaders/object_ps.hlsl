// Object pixel/fragment shader

#define PI 3.14159265

// 0 - base, 1 - normal, 2 - metallic, 3 - roughness, 4 - shadow
Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D roughnessTexture : register(t3);
TextureCube cubemapTexture : register(t4); // Environment map
Texture2D shadowMapTextureDirectional : register(t5);
TextureCube shadowMapTexture[4] : register(t6);
//TextureCube shadowMapTexture0 : register(t6);
//TextureCube shadowMapTexture1 : register(t7);
//TextureCube shadowMapTexture2 : register(t8);
//TextureCube shadowMapTexture3 : register(t9);
SamplerState SampleType : register(s0);
SamplerState SampleTypeClampPoint : register(s1);
//SamplerComparisonState SampleTypeComparison : register(s2);
SamplerComparisonState SampleTypeComparison : register(s2)
{
    Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;

    ComparisonFunc = LESS;
};
SamplerState SampleTypeCubemap : register(s3);

struct PointLight
{
    float4 lightDiffuseColour;
    float4 lightAmbientColour;
    float4 lightPositionAndRadius;
    float3 lightAttenuation;
    float isActive; // 0 - no light
};

// Light Buffer
cbuffer LightBuffer : register(cb0)
{
    PointLight pointLight[4];
    // Directional
    float4 lightDiffuseColour;
    float4 lightAmbientColour;
    float3 lightDirection;
    float padding;
};
cbuffer ShadowMapBuffer : register(cb1)
{
    float2 shadowMapSize;
    float directionalShadowMapQuality; // 0 - no shadows, 1 - hard shadows, 2 - soft shadows
    float pointShadowMapQuality;
};
cbuffer MaterialBuffer : register(cb2)
{
    float4 materialColour;
    float materialRoughness;
    float materialMetallic;
};

// Input
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : TEXCOORD1;
    float4 lightViewPosition : TEXCOORD2;
    float3 pos : TEXCOORD3;
};



// --------------- COLOUR (COOK-TORRANCE) --------------- //


// Fresnel reflectance(approximation by Shlick C.(1994))
float3 Fresnel(float halfVectorAngle, float3 F0)
{
    return F0 + (1 - F0) * pow(1.0f - halfVectorAngle, 5.0f);
}

// Distribution of microfacets

// This is used to check if we're calculating front of the microfacet(>1) or back(<=0) which shouldn't be visible
float chiGGX(float v)
{
    return v > 0 ? 1 : 0;
}
// This should give better results than Beckmann, especially for metal surfaces(artifacts appear when used with Cook-Torrance geometry shadowing)
float GGXDistribution(float halfVectorAngle, float roughness)
{
    float rougnessSquared = roughness * roughness;
    float halfVectorSquared = halfVectorAngle * halfVectorAngle;

    float den = halfVectorSquared * rougnessSquared + (1.0f - halfVectorSquared);

    return (chiGGX(halfVectorAngle) * rougnessSquared) / (PI * den * den);
}

float BeckmannDistribution(float halfVectorAngle, float roughness)
{
    float roughnessSquared = roughness * roughness;
    float halfVectorSquared = halfVectorAngle * halfVectorAngle;

    float numerator = 1.0f / (4.0f * roughnessSquared * pow(halfVectorAngle, 4.0f));
    float denominator = (halfVectorSquared - 1.0f) / (roughnessSquared * halfVectorSquared);

    return numerator * exp(denominator);
}

// Geometry function

float GGX_PartialGeometryTerm(float3 viewVector, float3 normalVector, float3 halfVector, float roughness)
{
    float viewHalfVectorAngle = saturate(dot(viewVector, halfVector));
    float chi = chiGGX(viewHalfVectorAngle / saturate(dot(viewVector, normalVector)));
    float viewHalfVectorAngleSquared = viewHalfVectorAngle * viewHalfVectorAngle;
    float tan2 = (1.0f - viewHalfVectorAngleSquared) / viewHalfVectorAngleSquared;
    return (chi * 2.0f) / (1.0f + sqrt(1.0f + roughness * roughness * tan2));
}

float CookTorranceGeometry(float halfVectorAngle, float lightVectorAngle, float lightHalfVectorAngle)
{
    float twoHalfVector = 2.0f * halfVectorAngle;

	// We will pick lower of the 2
    float halfVectorFunction = (twoHalfVector * halfVectorAngle) / lightHalfVectorAngle;
    float fullVectorFunction = (twoHalfVector * lightVectorAngle) / lightHalfVectorAngle;

    return min(1.0, min(halfVectorFunction, fullVectorFunction));
}

// PBR - Cook-Torrance

float3 CookTorrance(float3 materialDiffuseColor,
	float3 normal,
	float3 lightDir,
	float3 viewDir,
	float3 lightColor,
	float roughness,
	float metallic)
{
	// indices of refraction between air and shader object
    float ior = 0.1f;
    const float k = 0.1f; // error correction - isn't in formula but I added it to get rid of some problems appearing at low angles
    materialDiffuseColor += materialColour;
    materialDiffuseColor = saturate(materialDiffuseColor);
    roughness += materialRoughness;
    metallic += materialMetallic;
	// Calculating specular component
    float lightDirAngle = saturate(dot(normal, lightDir)) + k / 100;
    float3 Kd = 1.0f; // Ammount of light that is diffused
    float3 Ks = 0.0f; // Ammount of light that is specularly reflected
    if (lightDirAngle > 0)
    {
        float3 halfVector = normalize(lightDir + viewDir);
        float halfVectorAngle = saturate(dot(normal, halfVector));
        float viewDirAngle = saturate(dot(normal, viewDir));
		//float lightHalfVectorAngle = saturate(dot(lightDir, halfVector)); // Used in Cook-Torrance geometry function
		//float lightVectorAngle = saturate(dot(normal, lightDir));	// Used in Cook-Torrance geometry function

		// Calculate reflectance
        float3 F0 = abs((1.0 - ior) / (1.0 + ior)); // Actual formula to calculate that is ( (n1 - n2) / (n1 + n2) )^2 but here we use approximation
        F0 = F0 * F0;
        F0 = lerp(F0, materialDiffuseColor.rgb, metallic); // Shlick's formula is used only for materials like plastic, this line will help simulate metallic surfaces aswell
        float3 F = Fresnel(halfVectorAngle, F0);

		// Microfacet distribution
		//float D = BeckmannDistribution(halfVectorAngle, roughness);
        float D = GGXDistribution(halfVectorAngle, roughness);

		// Geometry function
		//float G = CookTorranceGeometry(halfVectorAngle, lightVectorAngle, lightHalfVectorAngle);
        float G = GGX_PartialGeometryTerm(viewDir, normal, halfVector, roughness);

		// Finally put it all together
        Ks = (D * F * G) / (PI * lightDirAngle * clamp(viewDirAngle, k, 1 - k));
		// Law of energy conservation, don't allow to reflect more light than comes in
        Kd = (1 - Ks);

    }


	// Calculate irradiance, this is simplified model using the same cubemap for both(spec and diff) components
    float3 envColor = cubemapTexture.Sample(SampleTypeCubemap, normal).xyz;
    float3 irradiance = envColor * saturate(metallic - roughness) + envColor * roughness;

    float3 diffuseComponent = materialDiffuseColor * irradiance * lightColor * lightDirAngle * Kd;
    float3 specularComponent = pow(lightColor, 2) * lightDirAngle * Ks;

    return diffuseComponent + specularComponent;
}


// --------------- COLOUR (COOK-TORRANCE) --------------- //



// --------------- SHADOWS (PCSS) --------------- //

float PercentageCloseFilter(float2 uv, float2 offset, float lightDepthValue, Texture2D shadowMap)
{
    float sum = 0.0f;
    const float PCF_STEP = 4;
    const float PCF_SUM = 81; //(PCF_STEP * 2 + 1) * (PCF_STEP * 2 + 1)

    for (int y = -PCF_STEP; y <= PCF_STEP; y++)
    {
        for (int x = -PCF_STEP; x <= PCF_STEP; x++)
        {
            float2 offsetUV = uv + float2(x, y) * offset;
            sum += shadowMap.SampleCmpLevelZero(SampleTypeComparison, offsetUV, lightDepthValue);
        }
    }

    return sum / PCF_SUM;
}


float CalculateShadow(float4 lightViewPosition, Texture2D shadowMap)
{
    const float BIAS = 0.0f;
    float3 projectTexCoord;
    float2 uv;
    float depthValue;
    float lightDepthValue;

    // --- NO SHADOWS --- //
    if (directionalShadowMapQuality == 0)
    {
        return 1;
    }

    // --- HARD SHADOWS --- //
    // Calculate pr0jected coordinates, then into UV range
    projectTexCoord.xyz = lightViewPosition.xyz / lightViewPosition.w;

	// Calculate the projected texture coordinates.
    uv.x = (projectTexCoord.x / 2.0f) + 0.5f;
    uv.y = (-projectTexCoord.y / 2.0f) + 0.5f;
    lightDepthValue = lightViewPosition.z / lightViewPosition.w;

    if (uv.x != saturate(uv.x) || uv.y != saturate(uv.y))
    {
        return 1;
    }

    lightDepthValue -= BIAS;

    if (directionalShadowMapQuality == 1)
    {
        // Something is wrong here, I can compare -999999 or 99999 instead of lightDepthValue and it doesn't make any difference
        return shadowMap.SampleCmp(SampleTypeComparison, uv, lightDepthValue);
    }
    else
    {
        // --- SOFT SHADOWS --- //
        float offset = float2(1.0 / shadowMapSize.x, 1.0 / shadowMapSize.y);
        // Change offset based on distance
        offset *= pow(1 + lightDepthValue, 2);

        return PercentageCloseFilter(uv, offset, lightDepthValue, shadowMap);
    }
}

float PercentageCloseFilterCubemap(float3 uvz, float3 offset, float lightDepthValue, TextureCube shadowCubemap)
{
    float sum = 0.0f;
    const float PCF_STEP = 3;
    const float PCF_SUM = 343; //(PCF_STEP * 2 + 1)^3

    for (int y = -PCF_STEP; y <= PCF_STEP; y++)
    {
        for (int x = -PCF_STEP; x <= PCF_STEP; x++)
        {
            for (int z = -PCF_STEP; z <= PCF_STEP; z++)
            {
                float3 offsetUVZ = uvz + float3(x, y, z) * offset;
                if (shadowCubemap.SampleLevel(SampleTypeCubemap, offsetUVZ, 0).z >= lightDepthValue)
                    sum += 1;
            }
        }
    }

    return sum / PCF_SUM;
}

// Borrowed from the internet but forgot where from :(
float ConvertDistToClipSpace(float3 lightDir_ws)
{
    float3 AbsVec = abs(lightDir_ws);
    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));
    float2 NearFar = float2(0.1f, 200); // Change later to pass that from CPU
    float NormZComp = (NearFar.y + NearFar.x) / (NearFar.y - NearFar.x)
        - (2.0f * NearFar.y * NearFar.x) / (LocalZcomp * NearFar.y - NearFar.x);

    return (NormZComp + 1) * 0.5f;
}

float CalculateShadowCubemap(float3 lightDir, float lightDist, TextureCube shadowCubemap)
{
    const float BIAS = 0.005f;
    float lightDepthValue;
   
    if (pointShadowMapQuality == 0)
    {
        return 1;
    }
    
    lightDepthValue = lightDist;
    lightDepthValue -= BIAS;
    lightDepthValue = ConvertDistToClipSpace(lightDir) - BIAS;

    if (pointShadowMapQuality == 1)
    {
        if (shadowCubemap.Sample(SampleTypeCubemap, lightDir).z >= lightDepthValue)
            return 1;
        else
            return 0;

    }
    else
    {
        float offset = float2(1.0 / shadowMapSize.x, 1.0 / shadowMapSize.y);
        // Change offset based on distance
        offset *= pow(8 + lightDepthValue, 2);

        return PercentageCloseFilterCubemap(lightDir, offset, lightDepthValue, shadowCubemap);
    }
}



float4 main(InputType input) : SV_TARGET
{
	// Textures
    Texture2D textureBase = baseTexture;
    Texture2D textureNormal = normalTexture;
    Texture2D textureMetallic = metallicTexture;
    Texture2D textureRoughness = roughnessTexture;
	// Textures samples for this fragment
    float4 baseColour = textureBase.Sample(SampleType, input.tex);
    float3 normalSample = textureNormal.Sample(SampleType, input.tex).rgb;
    float roughness = textureRoughness.Sample(SampleType, input.tex).r;
    float metallic = textureRoughness.Sample(SampleType, input.tex).r;
	// Light
    float lightIntensity = 0;
    float4 colour = float4(0, 0, 0, 0);
    float4 ambientColour = float4(0, 0, 0, 0);
    float4 lightingColour = float4(0, 0, 0, 0);
	// Shadows
	
    float shadow = 0.0f;

    // ----- NORMAL MAPPING ----- //
			// Move normal sample into the range of [-1, 1]
    normalSample = (normalSample * 2.0f) - 1.0f;
			// Calculate normal for this fragment
    float3 normal = (normalSample.x * input.tangent) + (normalSample.y * input.binormal) + (normalSample.z * input.normal);
			//normal = normalSample;
    normal = normalize(normal);
    //normal = input.normal;
	// ----- NORMAL MAPPING ----- //

    shadow = CalculateShadow(input.lightViewPosition, shadowMapTextureDirectional);
    
	// ----- LIGHTING ----- //
    // Point light
   // float3 lightDir = lightPosition - input.viewDirection;
    float3 lightDir = normalize(-lightDirection);

	// Final colour calculations

    ambientColour = saturate(baseColour * lightAmbientColour);

    if (shadow > 0)
    {
    
        lightingColour = float4(CookTorrance(baseColour, normal, lightDir, input.viewDirection, lightDiffuseColour, roughness, metallic), 1);
        lightingColour *= shadow;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (pointLight[i].isActive != 0)
        {
            lightDir = input.pos - pointLight[i].lightPositionAndRadius.xyz;
            float dist = length(lightDir);
            float range = pointLight[i].lightPositionAndRadius.w;
            if (dist < range)
            {
                // Shadow
                shadow = CalculateShadowCubemap(lightDir, dist, shadowMapTexture[i]);
                // Light
                if (shadow > 0)
                {
                    float attenuation = 1 / (pointLight[i].lightAttenuation.x + pointLight[i].lightAttenuation.y * dist + pointLight[i].lightAttenuation.z * pow(dist, 2));
                    lightingColour += float4(CookTorrance(float3(1, 1, 1), normal, -lightDir, input.viewDirection, pointLight[i].lightDiffuseColour, roughness, metallic), 0) * attenuation * shadow;
                }
            }
        }
    }
    colour = saturate(lightingColour + ambientColour);
	//colour = saturate(lightingColour + ambientColour);
	//colour = float4(depthValue, depthValue, depthValue, 1);
	//colour = specular;
	//colour = float4(normal, 1);
	//colour = float4(input.tex, 0, 1);
	//colour = float4(input.tangent, 1.0f);
   // colour = shadow;
    return colour;
}
























			/*

			BLINN PHONG

			float3 lightDir = input.viewDirection - lightPosition;
			float3 halfVector = normalize(lightDir + input.viewDirection);
			lightIntensity = saturate(dot(normal, halfVector));

			
			//lightIntensity = 1;	// testing shadows

			specular = 0;
			if (lightIntensity > 0)
			{
				// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
				float3 reflection = normalize(2 * lightIntensity * normal + lightPosition);

				// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
				float specularPower = 10;	// That should come from light
				specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
			}
			*/