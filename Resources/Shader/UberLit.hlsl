#pragma pack_matrix(row_major)

#define NUM_POINT_LIGHT 4
#define NUM_SPOT_LIGHT 4

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

cbuffer PerObject : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

cbuffer Lighting : register(b1)
{
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional;
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
};

float4 CalculateAmbientLight(FAmbientLightInfo info)
{
    float4 result;
    
    //...
    
    return result;
}

struct VS_INPUT
{
    float3 p : POSITIONT;
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
    output.position = float4(Input.p, 1.0);
    output.normal = Input.n;
    output.color = Input.c;
    output.uv = Input.t;
    output.worldPos = Input.p;
    
    
#if LIGHTING_MODEL_GOURAUD

#elif LIGHTING_MODEL_LAMBERT

#elif LIGHTING_MODEL_PHONG

#endif
    return output;
}

float4 Uber_PS(PS_INPUT Input) : SV_TARGET
{
    float4 finalPixel = Input.color;
    // finalPixel += Emissive;
#if LIGHTING_MODEL_GOURAUD
#elif LIGHTING_MODEL_LAMBERT
    //finalPixel += CalculateAmbientLight(...);
    //for(It : PointLights)
    //{
    //    finalPixel += CalculatePointLight(...);
    //}
#elif LIGHTING_MODEL_PHONG
    // Specular Reflectance
#endif
    return finalPixel;
}