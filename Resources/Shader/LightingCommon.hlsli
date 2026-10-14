#define TILE_SIZE 16
#define NUM_POINT_LIGHT 127
#define NUM_SPOT_LIGHT 127
#define TILE_SPOT_OFFSET (1 + NUM_POINT_LIGHT)
#define MAX_TILE_LIGHT_STRIDE 256

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