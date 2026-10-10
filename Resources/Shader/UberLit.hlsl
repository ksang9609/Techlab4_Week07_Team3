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
    float4 Ka;
    float4 Kd;
    float4 Ks;
    
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

float4 CalculateAmbientLight(FAmbientLightInfo info)
{
    float4 result;
    
    return result;
}

float4 CalculateDirectionalLight(FDirectionalLightInfo info, float3 normal, float3 viewDir)
{
    float4 result;
    
    return result;
}

float4 CalculatePointLight(FPointLightInfo info, float3 normal, float3 fragPos, float3 viewDir)
{
    float4 result;
    
    return result;
}

float4 CalculateSpotLight(FSpotLightInfo info, float3 normal, float3 fragPos, float3 viewDir)
{
    float4 result;
    
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
    float4 texColor = g_txDiffuse.Sample(g_Sample, Input.uv + UVOffset);
    
    float4 finalPixel = texColor;
    // finalPixel += Emissive;
#if LIGHTING_MODEL_GOURAUD
#elif LIGHTING_MODEL_LAMBERT
    //finalPixel += CalculateAmbientLight(Ambient);
    //for(It : PointLights)
    //{
    //    finalPixel += CalculatePointLight(...);
    //}
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
#endif
    return finalPixel;
}