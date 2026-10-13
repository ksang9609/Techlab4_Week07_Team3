#pragma pack_matrix(row_major)

#include "LightingCommon.hlsli"

// Light culling compute shader parameters
cbuffer LightCullParams : register(b0)
{
    matrix InverseViewProjection;
    int ScreenWidth;
    int ScreenHeight;
    int NumTileX;
    int NumTileY;
}

// Light constants buffer
cbuffer LightConstants : register(b4)
{
    float4 CameraPosition;
    FAmbientLightInfo Ambient;
    FDirectionalLightInfo Directional;
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
    int DirectionalLightCount;
    int PointLightCount;
    int SpotLightCount;
    int Light_Padding;
};

// Z-prepass 깊이 텍스처
Texture2D<float> SceneDepth  : register(t0);
// Light list를 저장할 RWStructuredBuffer
RWStructuredBuffer<uint> TileLightList : register(u0);

//최소/최대 Z값을 저장할 공유 변수
groupshared uint SharedMinZ;
groupshared uint SharedMaxZ;

// 살아남은 포인트 라이트 개수와 인덱스
groupshared uint SharedPointLightCount;
groupshared uint SharedPointLightIndices[NUM_POINT_LIGHT];

// 살아남은 스팟 라이트 개수와 인덱스
groupshared uint SharedSpotLightCount;
groupshared uint SharedSpotLightIndices[NUM_SPOT_LIGHT];

bool SphereIntersectsAABB(float3 Center, float Radius, float3 BoxMin, float3 BoxMax)
{
    // 박스에서 구 중심에 가장 가까운 점 찾기
    float3 ClosestPoint = clamp(Center, BoxMin, BoxMax);
    // 구 중심과 그 점 사이의 거리 제곱 구하기
    float3 Diff = ClosestPoint - Center;
    float Distance = dot(Diff, Diff);
    // 거리 제곱이 반지름 제곱보다 작거나 같으면 충돌
    return Distance <= (Radius * Radius);
}

struct CS_INPUT
{
    // 현재 속한 그룹의 좌표
    uint3 GroupId : SV_GroupID;
    // 화면 전체 기준으로 현재 픽셀의 절대 좌표
    uint2 DispatchThreadId : SV_DispatchThreadID;
    // 그룹(256) 안에서 현재 스레드의 인덱스
    int GroupIndex : SV_GroupIndex;
};

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(CS_INPUT input)
{
    // 그룹의 첫 번째 스레드에서 공유 변수 초기화
    if (input.GroupIndex == 0)
    {
        // 최대값으로 초기화
        SharedMinZ = 0x7F7FFFFF;
        SharedMaxZ = 0;
        SharedPointLightCount = 0;
        SharedSpotLightCount = 0;
    }
    // 모든 스레드가 공유 변수 초기화 완료될 때까지 대기
    GroupMemoryBarrierWithGroupSync();
    
    // 현재 스레드의 픽셀 좌표가 화면 범위 안에 있는지 확인
    if(input.DispatchThreadId.x < ScreenWidth && input.DispatchThreadId.y < ScreenHeight)
    {
        float Depth = SceneDepth.Load(int3(input.DispatchThreadId.xy, 0));
        // 깊이 값이 1.0보다 작으면 최소/최대 Z값 업데이트
        if (Depth < 1.0f)
        {
            // 깊이 값을 uint로 변환
            uint DepthUint = asuint(Depth);
            // 최소/최대 Z값 업데이트
            InterlockedMin(SharedMinZ, DepthUint);
            InterlockedMax(SharedMaxZ, DepthUint);
        }
    }
    // 모든 스레드가 최소/최대 Z값 업데이트 완료될 때까지 대기
    GroupMemoryBarrierWithGroupSync();
    
    float MinZ = asfloat(SharedMinZ);
    float MaxZ = asfloat(SharedMaxZ);
    
    // 최소/최대 Z값이 유효하면 타일의 AABB 계산
    if (MinZ <= MaxZ)
    {
        // 현재 타일의 픽셀 좌표 범위 계산
        float2 TileMinPx = float2(input.GroupId.xy * TILE_SIZE);
        float2 TileMaxPx = float2(min((input.GroupId.xy + 1) * TILE_SIZE, uint2(ScreenWidth, ScreenHeight)));
        
        // 타일의 UV 좌표 범위 계산
        float2 UVMin = TileMinPx / float2(ScreenWidth, ScreenHeight);
        float2 UVMax = TileMaxPx / float2(ScreenWidth, ScreenHeight);
        
        // 타일의 NDC 좌표 범위 계산
        float2 NDCMin = float2(UVMin.x * 2.0f - 1.0f, 1.0f - UVMin.y * 2.0f);
        float2 NDCMax = float2(UVMax.x * 2.0f - 1.0f, 1.0f - UVMax.y * 2.0f);

        float3 BoxMin = float3(1e30f, 1e30f, 1e30f);
        float3 BoxMax = float3(-1e30f, -1e30f, -1e30f);
        
        [unroll]
        for (uint i = 0; i < 8; ++i)
        {
            float3 CornerNDC;
            // NDC 좌표를 계산, 각 코너의 x, y, z 좌표를 결정
            CornerNDC.x = (i & 1) ? NDCMin.x : NDCMax.x;
            CornerNDC.y = (i & 2) ? NDCMin.y : NDCMax.y;
            CornerNDC.z = (i & 4) ? MinZ : MaxZ;

            // 월드 좌표를 계산
            float4 WorldH = mul(float4(CornerNDC, 1.0f), InverseViewProjection);
            float3 WorldPos = WorldH.xyz / WorldH.w;

            // 박스의 최소/최대 좌표 업데이트
            BoxMin = min(BoxMin, WorldPos);
            BoxMax = max(BoxMax, WorldPos);
        }

        if (input.GroupIndex < (uint) PointLightCount)
        {
            // 포인트 라이트의 중심과 반지름을 가져옴
            float3 Center = PointLights[input.GroupIndex].PositionRadius.xyz;
            float Radius = PointLights[input.GroupIndex].PositionRadius.w;
            
            // 포인트 라이트가 AABB와 교차하는지 확인
            if(SphereIntersectsAABB(Center, Radius, BoxMin, BoxMax))
            {
                // 중복 방지를 위한 변수
                uint Slot = 0;
                // 라이트가 AABB와 교차하면 관련된 라이트 수를 증가시킴
                InterlockedAdd(SharedPointLightCount, 1, Slot);
                // 라이트 인덱스를 저장
                SharedPointLightIndices[Slot] = input.GroupIndex;
            }
        }
        
        if (input.GroupIndex < (uint) SpotLightCount)
        {
            // 스팟 라이트의 중심과 반지름을 가져옴
            float3 Center = SpotLights[input.GroupIndex].PositionRadius.xyz;
            float Radius = SpotLights[input.GroupIndex].PositionRadius.w;
            
            // 스팟 라이트가 AABB와 교차하는지 확인
            if (SphereIntersectsAABB(Center, Radius, BoxMin, BoxMax))
            {
                // 중복 방지를 위한 변수
                uint Slot = 0;
                // 라이트가 AABB와 교차하면 관련된 라이트 수를 증가시킴
                InterlockedAdd(SharedSpotLightCount, 1, Slot);
                // 라이트 인덱스를 저장
                SharedSpotLightIndices[Slot] = input.GroupIndex;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    // 타일 인덱스 계산
    uint TileIndex = (input.GroupId.x + input.GroupId.y * NumTileX) * MAX_TILE_LIGHT_STRIDE;
    // 첫번째 스레드에서 살아남은 라이트 개수를 TileLightList에 저장
    if(input.GroupIndex == 0)
    {
        TileLightList[TileIndex + 0] = SharedPointLightCount;
        TileLightList[TileIndex + 17] = SharedSpotLightCount;
    }
    
    // 살아남은 라이트 인덱스를 TileLightList에 저장
    if (input.GroupIndex < SharedPointLightCount)
    {
        TileLightList[TileIndex + 1 + input.GroupIndex] = SharedPointLightIndices[input.GroupIndex];
    }
    if (input.GroupIndex < SharedSpotLightCount)
    {
        TileLightList[TileIndex + NUM_POINT_LIGHT + 2 + input.GroupIndex] = SharedSpotLightIndices[input.GroupIndex];
    }
}
