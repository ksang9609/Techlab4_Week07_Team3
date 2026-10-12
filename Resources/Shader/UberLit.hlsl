#pragma pack_matrix(row_major)

#define NUM_POINT_LIGHT 4
#define NUM_SPOT_LIGHT 4

Texture2D g_txAmbient : register(t0);
Texture2D g_txDiffuse : register(t1);
Texture2D g_txSpecular : register(t2);

SamplerState g_Sample : register(s0);

struct FAmbientLightInfo
{
    float3 Color; // RGB
    float Intensity;
};

struct FDirectionalLightInfo
{
    float3 Color;
    float Intensity;
    
    float3 Direction;
    float Padding;
};

struct FPointLightInfo
{
    float3 Color;
    float Intensity;
    
    float3 Position;
    float Radius;
    
    float LightFalloffExponent;
    float3 Padding;
};

struct FSpotLightInfo
{
    float3 Color;
    float Intensity;
    
    float3 Position;
    float Range;
    
    float3 Direction;
    float InnerConeCos;
    
    float OuterConeCos;
    float LightFalloffExponent;
    float2 Padding;
};

cbuffer Viewconstants : register(b0)
{
    matrix VP;
    float3 CameraPosition;
    float Padding;
};

cbuffer WorldConstants : register(b2)
{
    matrix World;
}

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float3 Ka;
    float Padding1;
    float3 Kd;
    float Padding2;
    float3 Ks;
    float Padding3;
    
    float2 UVOffset;
    float bOpaque;
    float Shininess;
};

cbuffer Lighting : register(b3)
{
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional;
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
};

// Lambert cosine-law reflection model
// Return C_l * I * max(0, N dot L)
float3 CalculateDiffuseReflection(float3 lightColor, float intensity, float NdotL)
{
    float3 diffuse = lightColor * intensity * saturate(NdotL);
    
    return diffuse;
}

// Blin-Phong reflection model
// Return C_s * I * max(0, N dot H)^shininess
float3 CalculateSpecularReflection(float3 lightColor, float intensity, float NdotL,
    float NdotH, float shininess)
{
    float3 specularFactor = pow(saturate(NdotH), shininess);
    
    specularFactor *= step(0, NdotL); // Only calculate specular if the light is in front of the surface
    
    float3 specular = lightColor * intensity * specularFactor;
    return specular;
}

float3 CalculateAmbientLight(FAmbientLightInfo info, float3 ambientColor)
{
    float3 result = ambientColor * info.Color * info.Intensity;
    return result;
}


float3 CalculateDirectionalDiffuse(float3 lightColor, float lightIntensity, float NdotL)
{
    float3 diffuse = CalculateDiffuseReflection(lightColor, lightIntensity, NdotL);
    return diffuse;
}

float3 CalculateDirectionalSpecular(float3 lightColor, float lightIntensity, float NdotL, float NdotH, float shininess)
{
    float3 specular = CalculateSpecularReflection(lightColor, lightIntensity, NdotL, NdotH, shininess);
    return specular;
}

// Calcuate attenuation based on distance
// Artistic attenuation model
// A(d) = [max(0, 1 - d / R)]^e
float CalculateArtisticAttenuation(float distance, float radius, float falloffExponent)
{
    return pow(saturate(1.0 - distance / radius), falloffExponent);
}

float3 CalculatePointDiffuse(float3 lightColor, float lightIntensity, float attenuation, float NdotL)
{
    float intensity = lightIntensity * attenuation;
    
    float3 diffuse = CalculateDiffuseReflection(lightColor, intensity, NdotL);
    return diffuse;
}

float3 CalculatePointSpecular(float3 lightColor, float lightIntensity, float attenuation, float NdotL, float NdotH, float shininess)
{
    float intensity = lightIntensity * attenuation;
    
    float3 specular = CalculateSpecularReflection(lightColor, intensity, NdotL, NdotH, shininess);
    return specular;
}

// Calculate cone attenuation for spot light
float CalculateConeAttenuation(float3 fragPos, float3 lightPos, float3 lightDir, float innerConeCos, float outerConeCos)
{
    float3 toSurface = normalize(fragPos - lightPos);
    float cosTheta = dot(toSurface, normalize(lightDir));
    
    float t = saturate(
        (cosTheta - outerConeCos) / max(innerConeCos - outerConeCos, 0.0001) // prevent zero-devision
    );
    
    float coneAttenuation = t * t * (3.0 - 2.0 * t); // smoothstep
    
    return coneAttenuation;
}

float3 CalculateSpotDiffuse(float3 lightColor, float lightIntensity, float distanceAttenuation, float coneAttenuation, float NdotL)
{
    float intensity = lightIntensity * distanceAttenuation * coneAttenuation;
    
    float3 diffuse = CalculateDiffuseReflection(lightColor, intensity, NdotL);
    return diffuse;
}

float3 CalculateSpotSpecular(float3 lightColor, float lightIntensity, float distanceAttenuation, float coneAttenuation, float NdotL, float NdotH, float shininess)
{
    float intensity = lightIntensity * distanceAttenuation * coneAttenuation;
    
    float3 specular = CalculateSpecularReflection(lightColor, intensity, NdotL, NdotH, shininess);
    return specular;
}

struct VS_INPUT
{
    float3 p : POSITION;
    float3 n : NORMAL;
    // Tangent, Bitangent, ...
    float4 c : COLOR;
    float2 t : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    
#if VIEW_MODE_NORMAL
    float3 worldNormal : NORMAL;
#elif LIGHTING_MODEL_LAMBERT || LIGHTING_MODEL_PHONG
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1; 
#elif LIGHTING_MODEL_GOURAUD
    float3 diffuseLighting : TEXCOORD2;
    float3 specularLighting : TEXCOORD3;
#endif
};

PS_INPUT Uber_VS(VS_INPUT Input)
{
    PS_INPUT output;
   
#if VIEW_MODE_NORMAL
    float4 worldPosition = mul(float4(Input.p, 1.0), World);
    output.position = mul(worldPosition, VP);
    output.worldNormal = normalize(mul(float4(Input.n, 0.0), World).xyz);
    output.color = Input.c;
    output.uv = Input.t;
#elif LIGHTING_MODEL_GOURAUD
    float4 worldPosition = mul(float4(Input.p, 1.0), World);
    output.position = mul(worldPosition, VP);
    float3 worldPos = worldPosition.xyz;
    
    // w = 0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    float3 normal = mul(float4(Input.n, 0.0), World).xyz;
    output.color = Input.c;
    output.uv = Input.t;
    
    // Calculate lighting in vertex shader for Gouraud shading
    float3 N = normalize(normal);
    float3 V = normalize(CameraPosition - worldPos);
    
    // Directional light
    {
        const float3 L = normalize(-Directional.Direction);
        const float NdotL = dot(N, L);
        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
    
        output.diffuseLighting = CalculateDirectionalDiffuse(Directional.Color, Directional.Intensity, NdotL);
        output.specularLighting = CalculateDirectionalSpecular(Directional.Color, Directional.Intensity, NdotL, NdotH, Shininess);
    }
    
    for (int i = 0; i < NUM_POINT_LIGHT; ++i)
    {
        if (PointLights[i].Intensity <= 0.0 ||
            PointLights[i].Radius <= 0.0)
        {
            continue;
        }
        
        const float3 L = normalize(PointLights[i].Position - worldPos);
        const float NdotL = dot(N, L);
        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
        const float distance = length(PointLights[i].Position - worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, PointLights[i].Radius, PointLights[i].LightFalloffExponent);
    
        output.diffuseLighting += CalculatePointDiffuse(PointLights[i].Color, PointLights[i].Intensity, distanceAttenuation, NdotL);
        output.specularLighting += CalculatePointSpecular(PointLights[i].Color, PointLights[i].Intensity, distanceAttenuation,  NdotL, NdotH, Shininess);
    }
    for (int i = 0; i < NUM_SPOT_LIGHT; ++i)
    {
        if (SpotLights[i].Intensity <= 0.0 ||
            SpotLights[i].Range <= 0.0)
        {
            continue;
        }
        
        const float3 L = normalize(SpotLights[i].Position - worldPos);
        const float NdotL = dot(N, L);
        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
        const float distance = length(SpotLights[i].Position - worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, SpotLights[i].Range, SpotLights[i].LightFalloffExponent);
        const float coneAttenuation = CalculateConeAttenuation(worldPos, SpotLights[i].Position, SpotLights[i].Direction, SpotLights[i].InnerConeCos, SpotLights[i].OuterConeCos);
    
        output.diffuseLighting += CalculateSpotDiffuse(SpotLights[i].Color, SpotLights[i].Intensity, distanceAttenuation, coneAttenuation, NdotL);
        output.specularLighting += CalculateSpotSpecular(SpotLights[i].Color, SpotLights[i].Intensity, distanceAttenuation, coneAttenuation, NdotL, NdotH, Shininess);
    }
    
#elif LIGHTING_MODEL_LAMBERT || LIGHTING_MODEL_PHONG
    output.worldPos = mul(float4(Input.p, 1.0), World).xyz;
    output.position = mul(mul(float4(Input.p, 1.0), World), VP);
    
    // w = 0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(Input.n, 0.0), World).xyz;
    output.color = Input.c;
    output.uv = Input.t;
#endif
    return output;
}

float4 Uber_PS(PS_INPUT Input) : SV_TARGET
{
    float4 finalPixel = float4(0.0, 0.0, 0.0, 0.0);
    // finalPixel += Emissive;
    float2 uv = Input.uv + UVOffset;
    
#if VIEW_MODE_NORMAL
    // (-1, 1) -> (0, 1)
    return float4(normalize(Input.worldNormal) * 0.5 + 0.5, 1.0);
#elif LIGHTING_MODEL_GOURAUD
    
    float3 ambientColor = g_txAmbient.Sample(g_Sample, uv).rgb * Ka;
    float3 diffuseColor = g_txDiffuse.Sample(g_Sample, uv).rgb * Kd;
    float3 specularColor = g_txSpecular.Sample(g_Sample, uv).rgb * Ks;
    
    finalPixel.rgb = 
        CalculateAmbientLight(Ambient, ambientColor) +
        Input.diffuseLighting * diffuseColor +
        Input.specularLighting * specularColor;
    
#elif LIGHTING_MODEL_LAMBERT
    
    const float3 ambientColor = g_txAmbient.Sample(g_Sample, uv).rgb * Ka;
    const float3 diffuseColor = g_txDiffuse.Sample(g_Sample, uv).rgb * Kd;
    
    const float3 N = normalize(Input.normal);
   
    // Ambient light
    finalPixel.rgb += CalculateAmbientLight(Ambient, ambientColor);

    // Directional light
    {
        const float3 L = normalize(-Directional.Direction);
        const float NdotL = dot(N, L);
        const float3 diffuseLightColor = CalculateDirectionalDiffuse(Directional.Color, Directional.Intensity, NdotL);

        finalPixel.rgb += diffuseColor * diffuseLightColor;
    }
    
    // Point lights
    for (int i = 0; i < PointLights.Length; i++)
    {
        if (PointLights[i].Intensity <= 0.0 || PointLights[i].Radius <= 0.0)
            continue;
    
        const float3 L = normalize(PointLights[i].Position - Input.worldPos);
        const float NdotL = dot(N, L);
        const float distance = length(PointLights[i].Position - Input.worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, PointLights[i].Radius, PointLights[i].LightFalloffExponent);
        const float3 diffuseLightColor = CalculatePointDiffuse(PointLights[i].Color, PointLights[i].Intensity, distanceAttenuation, NdotL);

        finalPixel.rgb += diffuseColor * diffuseLightColor;
    }
    
    // Spot lights
    for (int i = 0; i < SpotLights.Length; i++)
    {
        if (SpotLights[i].Intensity <= 0.0 || SpotLights[i].Range <= 0.0)
            continue;
        
        const float3 L = normalize(SpotLights[i].Position - Input.worldPos);
        const float NdotL = dot(N, L);
        const float distance = length(SpotLights[i].Position - Input.worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, SpotLights[i].Range, SpotLights[i].LightFalloffExponent);
        const float coneAttenuation = CalculateConeAttenuation(Input.worldPos, SpotLights[i].Position, SpotLights[i].Direction, SpotLights[i].InnerConeCos, SpotLights[i].OuterConeCos);
        const float3 diffuseLightColor = CalculateSpotDiffuse(SpotLights[i].Color, SpotLights[i].Intensity, distanceAttenuation, coneAttenuation, NdotL);

        finalPixel.rgb += diffuseColor * diffuseLightColor;
    }
#elif LIGHTING_MODEL_PHONG    
    const float3 ambientColor = g_txAmbient.Sample(g_Sample, uv).rgb * Ka;
    const float3 diffuseColor = g_txDiffuse.Sample(g_Sample, uv).rgb * Kd;
    const float3 specularColor = g_txSpecular.Sample(g_Sample, uv).rgb * Ks;
    
    const float3 N = normalize(Input.normal);
    const float3 V = normalize(CameraPosition - Input.worldPos);
   
    // Ambient light
    finalPixel.rgb += CalculateAmbientLight(Ambient, ambientColor);

    // Directional light
    {
        const float3 L = normalize(-Directional.Direction);
        const float NdotL = dot(N, L);
        const float3 diffuseLightColor = CalculateDirectionalDiffuse(Directional.Color, Directional.Intensity, NdotL);
    
        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
        const float3 specularLightColor = CalculateDirectionalSpecular(Directional.Color, Directional.Intensity, NdotL, NdotH, Shininess);

        finalPixel.rgb += diffuseColor * diffuseLightColor + specularColor * specularLightColor;
    }
    
    // Point lights
    for (int i = 0; i < PointLights.Length; i++)
    {
        if (PointLights[i].Intensity <= 0.0 || PointLights[i].Radius <= 0.0)
            continue;
    
        const float3 L = normalize(PointLights[i].Position - Input.worldPos);
        const float NdotL = dot(N, L);
        const float distance = length(PointLights[i].Position - Input.worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, PointLights[i].Radius, PointLights[i].LightFalloffExponent);
        const float3 diffuseLightColor = CalculatePointDiffuse(PointLights[i].Color, PointLights[i].Intensity, distanceAttenuation, NdotL);

        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
        const float3 specularLightColor = CalculatePointSpecular(PointLights[i].Color, PointLights[i].Intensity, distanceAttenuation, NdotL, NdotH, Shininess);

        finalPixel.rgb += diffuseColor * diffuseLightColor + specularColor * specularLightColor;
    }
    
    // Spot lights
    for (int i = 0; i < SpotLights.Length; i++)
    {
        if (SpotLights[i].Intensity <= 0.0 || SpotLights[i].Range <= 0.0)
            continue;
        
        const float3 L = normalize(SpotLights[i].Position - Input.worldPos);
        const float NdotL = dot(N, L);
        const float distance = length(SpotLights[i].Position - Input.worldPos);
        const float distanceAttenuation = CalculateArtisticAttenuation(distance, SpotLights[i].Range, SpotLights[i].LightFalloffExponent);
        const float coneAttenuation = CalculateConeAttenuation(Input.worldPos, SpotLights[i].Position, SpotLights[i].Direction, SpotLights[i].InnerConeCos, SpotLights[i].OuterConeCos);
        const float3 diffuseLightColor = CalculateSpotDiffuse(SpotLights[i].Color, SpotLights[i].Intensity, distanceAttenuation, coneAttenuation, NdotL);

        const float3 H = normalize(L + V);
        const float NdotH = dot(N, H);
        const float3 specularLightColor = CalculateSpotSpecular(SpotLights[i].Color, SpotLights[i].Intensity, distanceAttenuation, coneAttenuation, NdotL, NdotH, Shininess);

        finalPixel.rgb += diffuseColor * diffuseLightColor + specularColor * specularLightColor;
    }
#endif
    float alpha = bOpaque > 0.5 ? 1.0 : BaseColor.a;
    finalPixel.a = alpha;
    
    return finalPixel * BaseColor;
}