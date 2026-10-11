#pragma pack_matrix(row_major)

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

