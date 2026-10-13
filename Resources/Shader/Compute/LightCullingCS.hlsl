#pragma pack_matrix(row_major)

#define MAX_POINT_LIGHTS 128
#define MAX_SPOT_LIGHTS 128

cbuffer CullConstants : register(b0)
{
    float4x4 View;
    uint TileCountX;
    uint TileCountY;
    uint2 CullPadding;
}

struct FAmbientLightInfo
{
    float4 ColorIntensity;
};

struct FDirectionalLightInfo
{
    float4 Direction;
    float4 ColorIntensity;
};

struct FPointLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffParams; // x: LightFalloffExponent, y: bUseInverseSquaredFalloff
};

struct FSpotLightInfo
{
    float4 PositionRadius;
    float4 DirectionalAngle;
    float4 ColorIntensity;
    float4 ConeScaleOffset; // xy: Scale/Offset, z: LightFalloffExponent, w: bUseInverseSquaredFalloff
};

cbuffer LightConstants : register(b4)
{
    float4 CameraPosition;
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional;
    FPointLightInfo PointLights[MAX_POINT_LIGHTS];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHTS];
    int DirectionalLightCount;
    int PointLightCount;
    int SpotLightCount;
    int Light_Padding;
};

struct FTileFrustum
{
    float4 Planes[6];
};

struct FTileLightRange
{
    uint Offset;
    uint Count;
};

StructuredBuffer<FTileFrustum> TileFrustums : register(t0);

RWStructuredBuffer<FTileLightRange> OutPointRanges : register(u0);
RWStructuredBuffer<uint> OutPointIndices : register(u1);
RWStructuredBuffer<FTileLightRange> OutSpotRanges : register(u2);
RWStructuredBuffer<uint> OutSpotIndices : register(u3);

bool IsSphereInFrustum(
    float3 CenterVS,
    float Radius,
    FTileFrustum Frustum)
{
    [unroll]
    for (uint i = 0; i < 6; ++i)
    {
        float4 Plane = Frustum.Planes[i];

        if (dot(Plane.xyz, CenterVS) + Plane.w + Radius < 0.0f)
        {
            return false;
        }
    }

    return true;
}

[numthreads(8, 8, 1)]
void mainCS(uint3 ThreadID : SV_DispatchThreadID)
{
    uint2 TileXY = ThreadID.xy;

    if (TileXY.x >= TileCountX ||
        TileXY.y >= TileCountY)
    {
        return;
    }

    uint TileIndex = TileXY.y * TileCountX + TileXY.x;
    FTileFrustum Frustum = TileFrustums[TileIndex];

    uint PointOffset = TileIndex * MAX_POINT_LIGHTS;
    uint SpotOffset = TileIndex * MAX_SPOT_LIGHTS;

    uint PointCount = 0;
    uint SpotCount = 0;

    for (uint i = 0;
         i < min(PointLightCount, uint(MAX_POINT_LIGHTS));
         ++i)
    {
        float3 PositionVS = mul(
            float4(PointLights[i].PositionRadius.xyz, 1.0f),
            View).xyz;

        if (IsSphereInFrustum(
                PositionVS,
                PointLights[i].PositionRadius.w,
                Frustum))
        {
            OutPointIndices[PointOffset + PointCount] = i;
            ++PointCount;
        }
    }

    for (uint i = 0;
         i < min(SpotLightCount, uint(MAX_SPOT_LIGHTS));
         ++i)
    {
        float3 PositionVS = mul(
            float4(SpotLights[i].PositionRadius.xyz, 1.0f),
            View).xyz;

        if (IsSphereInFrustum(
                PositionVS,
                SpotLights[i].PositionRadius.w,
                Frustum))
        {
            OutSpotIndices[SpotOffset + SpotCount] = i;
            ++SpotCount;
        }
    }

    FTileLightRange PointRange;
    PointRange.Offset = PointOffset;
    PointRange.Count = PointCount;
    OutPointRanges[TileIndex] = PointRange;

    FTileLightRange SpotRange;
    SpotRange.Offset = SpotOffset;
    SpotRange.Count = SpotCount;
    OutSpotRanges[TileIndex] = SpotRange;
}