#pragma pack_matrix(row_major)

cbuffer FrustumBuildConstants : register(b0)
{
    float4x4 Projection;
    
    uint ViewportWidth;
    uint ViewportHeight;
    uint TileSize;
    uint TileCountX;
    
    uint TileCountY;
    uint3 Padding;
}

struct FTileFrustum
{
    // xyz: normal, w: plane distance
    float4 Planes[6];
};

RWStructuredBuffer<FTileFrustum> OutFrustums : register(u0);

float4 NormalizePlane(float4 Plane)
{
    return Plane / length(Plane.xyz);
}

[numthreads(8, 8, 1)]
void mainCS(uint3 ThreadID : SV_DispatchThreadID)
{
    const uint2 TileXY = ThreadID.xy;
    
    if (TileXY.x >= TileCountX ||
        TileXY.y >= TileCountY)
    {
        return;
    }
    
    const uint2 PixelMin = TileXY * TileSize;
    const uint2 PixelMax = min(
        PixelMin + uint2(TileSize, TileSize),
        uint2(ViewportWidth, ViewportHeight));

    // Convert pixel coordinates to normalized device coordinates (NDC)
    // X: [0, ViewportWidth] -> [-1, 1] (left to right)
    // Y: [0, ViewportHeight] -> [1, -1] (top to bottom)
    const float MinX = 2.0f * float(PixelMin.x) / ViewportWidth - 1.0f;
    const float MaxX = 2.0f * float(PixelMax.x) / ViewportWidth - 1.0f;
    
    const float MinY = 1.0f - 2.0f * float(PixelMax.y) / ViewportHeight;
    const float MaxY = 1.0f - 2.0f * float(PixelMin.y) / ViewportHeight;

    // Extract the columns of the projection matrix
    const float4 Column0 = float4(
        Projection[0][0], Projection[1][0],
        Projection[2][0], Projection[3][0]
    );
    const float4 Column1 = float4(
        Projection[0][1], Projection[1][1],
        Projection[2][1], Projection[3][1]
    );
    const float4 Column2 = float4(
        Projection[0][2], Projection[1][2],
        Projection[2][2], Projection[3][2]
    );
    const float4 Column3 = float4(
        Projection[0][3], Projection[1][3],
        Projection[2][3], Projection[3][3]
    );

    FTileFrustum Frustum;
    
    Frustum.Planes[0] = NormalizePlane( Column0 - MinX * Column3);  // Left
    Frustum.Planes[1] = NormalizePlane(-Column0 + MaxX * Column3);  // Right
    Frustum.Planes[2] = NormalizePlane( Column1 - MinY * Column3);  // Bottom
    Frustum.Planes[3] = NormalizePlane(-Column1 + MaxY * Column3);  // Top
    Frustum.Planes[4] = NormalizePlane(Column2);                    // Near
    Frustum.Planes[5] = NormalizePlane(Column3 - Column2);          // Far

    const uint TileIndex = TileXY.y * TileCountX + TileXY.x;
    OutFrustums[TileIndex] = Frustum;
}

