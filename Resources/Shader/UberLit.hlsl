#pragma pack_matrix(row_major)
#define NUM_POINT_LIGHT 16
#define NUM_SPOT_LIGHT 16

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
    float4 FalloffEnabled;
    float4 LightDirection;
};

cbuffer Viewconstants : register(b0)
{
    matrix VP;
    float4 CamPos;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float2 UVOffset;
    float bOpaque;
    float Padding;
};

struct FireBallLight
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
};

cbuffer FireBallLightConstants : register(b3)
{
    FireBallLight FireBalls[16];
    uint FireBallCount;
    float3 FireBallLightPadding;
};

struct Light
{
    float4 PositionRadius;
    float4 ColorIntensity;
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
    int NumDirectionalLight;
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
    LightVec = normalize(LightVec);
    ToEye= normalize(ToEye);
    
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

float3 CalculateSpotLight(FSpotLightInfo L, float3 Pos, float3 Normal)
{
    float3 Direction = L.LightDirection.xyz;
    float3 LightVec = L.PositionRadius.xyz - Pos;
    float CosInner = cos(radians(L.FalloffEnabled.z));
    float CosOuter = cos(radians(L.FalloffEnabled.w));
    
    float D = length(LightVec);
    if (D > L.PositionRadius.w)
    {
        return 0.0f;
    }
        
    LightVec /= D;
    
    Direction = normalize(Direction);
    LightVec = normalize(LightVec);
    
    float Cos = dot(Direction, -LightVec);

    float SpotFactor = saturate((Cos - CosOuter) / (CosInner - CosOuter));
    
    float Radius = L.PositionRadius.w;
    float RadiusFalloff = L.FalloffEnabled.x;
    
    float Intensity = L.ColorIntensity.w;
    float3 LightColor = L.ColorIntensity.xyz;
    
    float NdotL = max(dot(LightVec, Normal), 0.0f);
    
    float3 LightStrength = Intensity * DiffuseAlbedo * LightColor * NdotL;
    LightStrength *= CalcAttenuation(D, RadiusFalloff, Radius);
    LightStrength *= SpotFactor;
    
    return LightStrength;
}


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
    
#if LIGHTING_MODEL_UNLIT
    return Output;
    
#endif
    
#if LIGHTING_MODEL_GOURAUD
    // 환경광, 정반사, 난반사 전부다 전달
    float4 DiffuseColor = float4(0, 0, 0, 0);
    float4 SpecularColor = float4(0, 0, 0, 0);
    
    //float4 TexColor = g_txColor.SampleLevel(g_Sample, Input.TexCoord + UVOffset, 0.0f);
    //float4 Color = TexColor * BaseColor;
    //float4 AmbientColor = CalculateAmbientLight(Ambient, Color);
    
    if (NumDirectionalLight)
    {
        float3 Diffuse = CalculateDirectionalLight(Directional, Output.NormalW);
        DiffuseColor += float4(Diffuse, 0);
        float3 LightVec = -Directional.LightDirection;
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Output.NormalW, ToEye), 0);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
        float3 Diffuse = CalculatePointLight(PointLights[i], Output.PosW, Output.NormalW);
        DiffuseColor += float4(Diffuse, 0);
        float3 LightVec = PointLights[i].PositionRadius.xyz - Output.PosW;   
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Output.NormalW, ToEye), 0);
    }
    
    for (int i = 0; i < NumSpotLight; i++)
    {
        float3 Diffuse = CalculateSpotLight(SpotLights[i], Output.PosW, Output.NormalW);
        DiffuseColor += float4(Diffuse, 0);
        float3 LightVec = SpotLights[i].PositionRadius.xyz - Output.PosW;  
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Output.NormalW, ToEye), 0);
    }
    
    //float4 FinalColor = AmbientColor + DiffuseColor + SpecularColor;

    float4 FinalColor = DiffuseColor + SpecularColor;
    
    Output.Color = FinalColor;
    Output.NormalW = FinalColor.xyz;
    return Output;
#endif
    

#if LIGHTING_MODEL_LAMBERT
    
#endif
    
#if LIGHTING_MODEL_PHONG

#endif
    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
    float4 FinalPixel = float4(0, 0, 0, 0);
    //finalPixel += Emissive;
    
    float4 TexColor = g_txColor.Sample(g_Sample, Input.UV + UVOffset);
    float4 Color = TexColor * BaseColor;
    //float4 AmbientColor = Color * float4(0.5f, 0.5f, 0.5f, 1.0f);
    float4 AmbientColor = CalculateAmbientLight(Ambient, Color);
    
    float4 DiffuseColor = float4(0, 0, 0, 0);
    float4 SpecularColor = float4(0, 0, 0, 0);
        
#if LIGHTING_MODEL_UNLIT
    return Color;
#endif
    
#if LIGHTING_MODEL_GOURAUD
    FinalPixel = AmbientColor + Input.Color;
    FinalPixel.w = 1.0f;
    return FinalPixel;

#endif
    
#if LIGHTING_MODEL_LAMBERT
    if (NumDirectionalLight)
    {
        DiffuseColor += float4(CalculateDirectionalLight(Directional, Input.NormalW), 0.0f);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
        DiffuseColor += float4(CalculatePointLight(PointLights[i], Input.PosW, Input.NormalW), 0.0f);
    }
    
    for (int i = 0; i < NumSpotLight; i++)
    {
        DiffuseColor += float4(CalculateSpotLight(SpotLights[i], Input.PosW, Input.NormalW), 0.0f);
    }
    
#endif
    
#if LIGHTING_MODEL_PHONG
    // Specular Reflectance
    float3 ToEye = CamPos.xyz - Input.PosW;
    
    if (NumDirectionalLight)
    {
        float3 Diffuse = CalculateDirectionalLight(Directional, Input.NormalW);
        float3 LightVec = -Directional.LightDirection;
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye), 0);
    }
    
    for (int i = 0; i < NumPointLight; i++)
    {
        float3 Diffuse = CalculatePointLight(PointLights[i], Input.PosW, Input.NormalW);
        float3 LightVec = PointLights[i].PositionRadius.xyz - Input.PosW;   
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye), 0);
    }
    
    for (int i = 0; i < NumSpotLight; i++)
    {
        float3 Diffuse = CalculateSpotLight(SpotLights[i], Input.PosW, Input.NormalW);
        float3 LightVec = SpotLights[i].PositionRadius.xyz - Input.PosW; 
        SpecularColor += float4(BlinnPhong(Diffuse, LightVec, Input.NormalW, ToEye), 0);
    }
    
#endif
    FinalPixel = AmbientColor + DiffuseColor + SpecularColor;
    return FinalPixel;
}