#pragma pack_matrix(row_major)
#include "Constants.hlsli"
#define NUM_POINT_LIGHT 4
#define NUM_SPOT_LIGHT 4

struct FAmbientLightInfo
{
    float4 ColorIntensity;
};

struct FDirectionalLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 LightDirection;
};

struct FPointLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
};

struct FSpotLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 LightDirection;
    float4 FalloffEnabled;
};

cbuffer Lighting : register(b4)
{
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional;
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
    int NumPointLight;
    int NumSpotLight;
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

// 임시 하드코딩 반사계수
static const float3 DiffuseAlbedo = float3(0.5f, 0.5f, 0.5f);
static const float3 SpecularAlbedo = float3(0.5f, 0.5f, 0.5f);
static const float Shininess = 0.125;

float4 CalculateAmbientLight(FAmbientLightInfo Info, float4 Color)
{
    float4 Result;
    Result = float4(Info.ColorIntensity.xyz * Info.ColorIntensity.w, 1.0f) * Color;
    
    return Result;
}

float CalcAttenuation(float D, float FallOffStart, float FallOffEnd)
{
    return saturate((FallOffEnd - D) / (FallOffEnd - FallOffStart));
}

/***
LightStrength : Intensity * LightColor
***/
    float3 BlinnPhong(float3 LightStrength, float3 LightVec, float3 Normal, float3 ToEye)
{ 
    float3 Specular = float3(0, 0, 0);
    

    // 직접 조명이 없으면 반사광도 없음
   if (all(LightStrength == 0.0f))
    {
        return Specular;
    }

    float3 HalfSum = ToEye + LightVec;    
    float HalfLengthSq = dot(HalfSum, HalfSum);

    // 빛 방향과 시선 방향이 반대면 합이 0이 될 수 있음
    if (HalfLengthSq < 1e-8f)
    {
        return Specular;
    }

    float3 HalfVec = HalfSum * rsqrt(HalfLengthSq);

    float M = max(Shininess * 256.0f, 1.0f);
    float NdotH = saturate(dot(HalfVec, Normal));

    // pow(0, M) 계산 경로를 피함
    float RoughnessFactor = 0.0f;
    if (NdotH > 0.0f)
    {
        RoughnessFactor = (M + 8.0f) * pow(NdotH, M) / 8.0f;
    }
    
    float3 SpecFactor = SpecularAlbedo * RoughnessFactor;
    SpecFactor = SpecFactor / (SpecFactor + 1.0f);
    
    Specular =  SpecFactor * LightStrength;
    
    return Specular;
}


float3 CalculateDirectionalLight(FDirectionalLightInfo L, float3 Normal)
{
    float3 LightVec = -L.LightDirection;
    LightVec = normalize(LightVec);
    
    float Intensity = L.ColorIntensity.w;
    float3 LightColor = L.ColorIntensity.xyz;
    
    // Lambert's Cosine Law
    float NdotL = max(dot(LightVec, Normal), 0.0f);
    
    float3 LightStrength = Intensity * DiffuseAlbedo * LightColor * NdotL;
    return LightStrength;
}

float3 CalculatePointLight(FPointLightInfo L, float3 Pos, float3 Normal)
{
    float3 LightVec = L.PositionRadius.xyz - Pos;
    float D = length(LightVec);
    if (D > L.PositionRadius.w)
    {
        return 0.0f;
    }
    
    LightVec /= D;
    
    float Radius = L.PositionRadius.w;
    float RadiusFalloff = L.FalloffEnabled.x;
    
    float Intensity = L.ColorIntensity.w;
    float3 LightColor = L.ColorIntensity.xyz;
    
    float NdotL = max(dot(LightVec, Normal), 0.0f);
    
    float3 LightStrength = Intensity * DiffuseAlbedo * LightColor * NdotL;
    LightStrength *= CalcAttenuation(D, RadiusFalloff, Radius);
    
    return LightStrength;
}

//float3 CalculateSpotLight(SpotLight L, Material Mat, float3 Pos, float3 Normal, float3 ToEye)
//{
//    float3 LightVec = L.Position - Pos;
//    float D = length(LightVec);
//    if (D > L.FallOffEnd)
//    {
//        return 0.0f;
//    }
    
//    LightVec /= D;
    
//    float NdotL = max(dot(LightVec, Normal), 0.0f);
//    float3 LightStrength = L.Intensity * L.LightColor * NdotL;
    
//    float Att = CalcAttenuation(D, L.FallOffStart, L.FallOffEnd);
    
//    float SpotFactor = pow(max(1 - LightVec, L.LightDirection), L.SpotPower);
//    LightStrength *= Att * SpotFactor;
    
//    return BlinnPhong(LightStrength, LightVec, Normal, ToEye, Mat);
//}


struct VS_INPUT
{
    float3 PosL : POSITION; // Input position from vertex buffer
    float3 NormalL : NORMAL;
    float4 Color : COLOR; // Input color from vertex buffer
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 NormalW : NORMAL;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float3 PosW : TEXCOORD1;
};

PS_INPUT mainVS(VS_INPUT Input)
{
    PS_INPUT Output;
    
    Output.PosW = mul(float4(Input.PosL, 1.0f), World).xyz;
    Output.PosH = mul(mul(float4(Input.PosL, 1.0f), World), VP);
    Output.NormalW = mul(float4(Input.NormalL, 0.0f), World).xyz;
    Output.Color = Input.Color;
    Output.UV = Input.TexCoord;
    
    float3 ToEye = CamPos.xyz - Output.PosW;
    
#if LIGHTING_MODEL_GOURAUD
    // 환경광, 정반사, 난반사 전부다 전달
    float4 DiffuseColor = float4(0, 0, 0, 0);
    float4 SpecularColor = float4(0, 0, 0, 0);
    
    float4 TexColor = g_txColor.Sample(g_Sample, Input.TexCoord + UVOffset);
    float4 Color = TexColor * BaseColor;
    float4 AmbientColor = CalculateAmbientLight(Ambient, Color);
    // float3 ToEye = CamPos - Input.PosW;
    
    if (Directional)
    {
        float3 Diffuse = CalculateDirectionalLight(Directional, Output.NormalW);
        DiffuseColor += Diffuse;
        float3 LightVec = -Directional.LightDirection;
        SpecularColor += BlinnPhong(Diffuse, LightVec, Output.NormalW, ToEye);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
         CalculatePointLight(PointLights[i], Output.PosW, Output.NormalW);
    
        float3 Diffuse = CalculatePointLight(PointLights[i], Output.PosW, Input.NormalW);
        DiffuseColor += Diffuse;
        float3 LightVec = PointLights[i].PositionRadius.xyz - Output.PosW;   
        SpecularColor += BlinnPhong(Diffuse, LightVec, Output.NormalW, ToEye);
    }
    
    //for (int i = 0; i < NumSpotLight; i++)
    //{
    //    DiffuseColor += CalculateSpotLight(SpotLights[i], Output.PosW, Output.NormalW);
    //}
    
    float4 FinalColor = AmbientColor + DiffuseColor; // + SpecularColor
    
    Output.Color = FinalColor;

#elif LIGHTING_MODEL_LAMBERT

#elif LIGHTING_MODEL_PHONG
    
#endif
    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
    float4 FinalPixel;
    //finalPixel += Emissive;
    
    float4 TexColor = g_txColor.Sample(g_Sample, Input.UV + UVOffset);
    float4 Color = TexColor * BaseColor;
    float4 AmbientColor = CalculateAmbientLight(Ambient, Color);
    float4 DiffuseColor = float4(0, 0, 0, 0);
    float4 SpecularColor = float4(0, 0, 0, 0);
    
#if LIGHTING_MODEL_GOURAUD
    AmbientColor = float4(0, 0, 0, 0);
    FinalPixel = Input.Color;
    
#elif LIGHTING_MODEL_LAMBERT
    
    if (Directional)
    {
        DiffuseColor += CalculateDirectionalLight(Directional, Output.NormalW);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
        DiffuseColor += CalculatePointLight(PointLights[i], Output.PosW, Output.NormalW);
    }
    
    //for (int i = 0; i < NumSpotLight; i++)
    //{
    //    DiffuseColor += CalculateSpotLight(SpotLights[i], Output.PosW, Output.NormalW);
    //}
    
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
    float3 ToEye = CamPos.xyz - Input.PosW;
    
    if (Directional)
    {
        float3 Diffuse = CalculateDirectionalLight(Directional, Input.NormalW);
        float3 LightVec = -Directional.LightDirection;
        SpecularColor += BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
        float Diffuse3 = CalculatePointLight(PointLights[i], Input.PosW, Input.NormalW);
        float3 LightVec = PointLights[i].PositionRadius.xyz - Input.PosW;   
        SpecularColor += BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye);
    }
    
    //for (int i = 0; i < NumSpotLight; i++)
    //{
    //    DiffuseColor += CalculateSpotLight(SpotLights[i], Input.PosW, Input.NormalW);
    //    //SpecularColor += BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye);
    //}
    
#endif
    
    FinalPixel = AmbientColor + DiffuseColor + SpecularColor;
    return FinalPixel;
}