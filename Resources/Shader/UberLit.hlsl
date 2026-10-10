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
    float Padding[3];
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
    float Padding[3];
};

cbuffer Viewconstants : register(b0)
{
    matrix VP;
};

cbuffer WorldConstants : register(b2)
{
    matrix World;
}

cbuffer MaterialParams : register(b1)
{
    float3 Ka;
    float Dissolve;
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

float3 CalculateDiffuseReflection(float3 diffuseColor, float3 lightColor, float intensity, float3 lightDir, float3 normal)
{    
    float NdotL = saturate(dot(normal, lightDir));
    float3 diffuse = diffuseColor * lightColor * intensity * NdotL;
    
    return diffuse;
}  

float3 CalculateAmbientLight(FAmbientLightInfo info, float3 ambientColor)
{
    float3 result = ambientColor * info.Color * info.Intensity;
    return result;
}

float3 CalculateDirectionalLight(FDirectionalLightInfo info, float3 normal, float3 viewDir,
    float3 diffuseColor, float3 specularColor)
{
    float3 result = float3(0.0, 0.0, 0.0);
    
#if LIGHTING_MODEL_LAMBERT
    float3 L = normalize(-info.Direction);
    float3 N = normalize(normal);
    
    float3 diffuse = CalculateDiffuseReflection(diffuseColor, info.Color, info.Intensity, L, N);
    result += diffuse;
    
#elif LIGHTING_MODEL_PHONG
#endif
    
    return result;
}

float3 CalculatePointLight(FPointLightInfo info, float3 normal, float3 fragPos, float3 viewDir,
    float3 diffuseColor, float3 specularColor)
{
    float3 result = float3(0.0, 0.0, 0.0);
    
#if LIGHTING_MODEL_LAMBERT
    float3 L = normalize(info.Position - fragPos);
    float3 N = normalize(normal);
    
    // Calcuate attenuation based on distance
    // Artistic attenuation model
    // A(d) = [max(0, 1 - d / R)]^e
    float distance = length(info.Position - fragPos);
    float attenuation = pow(saturate(1.0 - distance / info.Radius), info.LightFalloffExponent);
    float intensity = info.Intensity * attenuation;
    
    float3 diffuse = CalculateDiffuseReflection(diffuseColor, info.Color, intensity, L, N);
    result += diffuse;
    
#elif LIGHTING_MODEL_PHONG
#endif
    
    return result;
}

float3 CalculateSpotLight(FSpotLightInfo info, float3 normal, float3 fragPos, float3 viewDir)
{
    float3 result = float3(0.0, 0.0, 0.0);
    
    //...
    
    return result;
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
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

PS_INPUT Uber_VS(VS_INPUT Input)
{
    PS_INPUT output;
    
#if LIGHTING_MODEL_GOURAUD
    output.worldPos = mul(float4(Input.p, 1.0), World).xyz;
    output.position = mul(mul(float4(Input.p, 1.0), World), VP);
    
    // w = 0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(Input.n, 0.0), World).xyz;
    output.color = Input.c;
    output.uv = Input.t;
#elif LIGHTING_MODEL_LAMBERT
    output.worldPos = mul(float4(Input.p, 1.0), World).xyz;
    output.position = mul(mul(float4(Input.p, 1.0), World), VP);
    
    // w = 0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(Input.n, 0.0), World).xyz;
    output.color = Input.c;
    output.uv = Input.t;
#elif LIGHTING_MODEL_PHONG
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
#if LIGHTING_MODEL_GOURAUD
#elif LIGHTING_MODEL_LAMBERT
    //finalPixel += CalculateAmbientLight(Ambient);
    //for(It : PointLights)
    //{
    //    finalPixel += CalculatePointLight(...);
    //}
    float3 ambientColor = g_txAmbient.Sample(g_Sample, Input.uv + UVOffset).rgb * Ka;
    float3 diffuseColor = g_txDiffuse.Sample(g_Sample, Input.uv + UVOffset).rgb * Kd;
    float3 specularColor = float3(0.0, 0.0, 0.0);
    
    // Lambert reflection model doesn't have specular reflection, so we can ignore it for now.
    float3 viewDir = float3(0.0, 0.0, 0.0);
    
    float3 N = normalize(Input.normal);
    
    finalPixel.rgb += CalculateAmbientLight(Ambient, ambientColor);
    finalPixel.rgb += CalculateDirectionalLight(Directional, N, viewDir, diffuseColor, specularColor);
    for (int i = 0; i < PointLights.Length; i++)
    {
        if (PointLights[i].Intensity <= 0.0 || PointLights[i].Radius <= 0.0)
            continue;
    
        finalPixel.rgb += CalculatePointLight(PointLights[i], N, Input.worldPos, viewDir, diffuseColor, specularColor);
    }
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
#endif
    float alpha = bOpaque > 0.5 ? 1.0 : Dissolve;
    finalPixel.a = alpha;
    
    return finalPixel;
}