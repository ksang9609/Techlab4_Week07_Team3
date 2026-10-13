#pragma pack_matrix(row_major)

#define NUM_POINT_LIGHT 128
#define NUM_SPOT_LIGHT 128

cbuffer Viewconstants : register(b0)
{
    matrix VP;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float3 Ka;
    float Padding1;
    float3 Kd;
    float Padding2;
    float3 Ks;
    float Padding3;

    float2 UVOffset;
    float bOpaque;
    float SpecularPower;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
    matrix WorldInverseTranspose;
};

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
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
    int DirectionalLightCount;
    int PointLightCount;
    int SpotLightCount;
    int Light_Padding;
};

cbuffer TileLightConstants : register(b5)
{
    uint TileSize;
    uint TileCountX;
    uint TileCountY;
    uint TileLightEnabled;
    
    uint ViewportStartX;
    uint ViewportStartY;
    uint2 Padding;
};

struct FTileLightRange
{
    uint Offset;
    uint Count;
};

StructuredBuffer<FTileLightRange> TilePointLightRanges : register(t8);
StructuredBuffer<uint> TilePointLightIndices : register(t9);
StructuredBuffer<FTileLightRange> TileSpotLightRanges : register(t10);
StructuredBuffer<uint> TileSpotLightIndices : register(t11);

uint GetTileIndex(float2 ScreenPos)
{
    float2 LocalPosition = max(
        ScreenPos - float2(ViewportStartX, ViewportStartY),
        float2(0.0f, 0.0f)
    );

    uint2 TileXY = uint2(LocalPosition) / TileSize;
    
    TileXY = min(
        TileXY,
        uint2(TileCountX - 1, TileCountY - 1)
    );
    
    return TileXY.y * TileCountX + TileXY.x;
}

// (1 ~ 7 ~ 15) -> (Blue ~ Green ~ Red)
float3 GetLightCountColor(uint Count)
{
    if (Count == 0)
    {
        return float3(0, 0, 0);
    }
    
    
    float T = saturate((float(Count) - 1.0f) / 15.0f);

    if (T < 0.5f)
    {
        return lerp(
            float3(0.0f, 0.0f, 1.0f),
            float3(0.0f, 1.0f, 0.0f),
            T * 2.0f);
    }
    
    return lerp(
        float3(0.0f, 1.0f, 0.0f),
        float3(1.0f, 0.0f, 0.0f),
        (T - 0.5f) * 2.0f);
}

float4 VisualizeTileLightCount(float2 ScreenPos)
{
    if (TileLightEnabled == 0 ||
        TileSize == 0 ||
        TileCountX == 0 ||
        TileCountY == 0)
    {
        return float4(1.0f, 0.0f, 1.0f, 1.0f);
    }
    
    uint TileIndex = GetTileIndex(ScreenPos);
    
    uint Count = 
        TilePointLightRanges[TileIndex].Count +
        TileSpotLightRanges[TileIndex].Count;
    
    return float4(GetLightCountColor(Count), 1.0f);
}

struct VS_INPUT
{
    float3 p : POSITION; // Input position from vertex buffer
    float3 n : NORMAL;
    float4 c : COLOR; // Input color from vertex buffer
    float2 t : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;

#if VIEW_MODE_NORMAL
    float3 worldNormal : NORMAL;
#elif LIGHTING_MODEL_LAMBERT || LIGHTING_MODEL_PHONG
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
#elif LIGHTING_MODEL_GOURAUD
    float3 diffuseLighting : TEXCOORD2;
    float3 specularLighting : TEXCOORD3;
#endif
};

Texture2D g_txAmbient : register(t0);
Texture2D g_txDiffuse : register(t1);
Texture2D g_txSpecular : register(t2);
SamplerState g_Sample : register(s0);

float3 CalculateAmbientLight(FAmbientLightInfo info)
{
    // Ambient Light 계산
    return info.ColorIntensity.rgb * info.ColorIntensity.a;
}

float BlinnPhong(float3 N, float3 L, float3 V, float SpecularPower)
{
    // 하프 벡터 계산
    float3 H = normalize(L + V);
    // N과 H의 내적 계산
    float NdotH = saturate(dot(N, H));
    // 블린퐁 반사광 계산
    return pow(NdotH, SpecularPower);
}

void DefaultLit(float3 N, float3 V, float3 L, float3 LightRadiance, float SpecularPower, out float3 OutDiffuse, out float3 OutSpecular)
{
    // 람베르트 산란광
    float NdotL = saturate(dot(N, L));
    OutDiffuse = NdotL * LightRadiance;
    OutSpecular = float3(0.0f, 0.0f, 0.0f);
    // 빛이 표면 뒤쪽에 있으면 반사광 계산을 하지 않는다.
#if !LIGHTING_MODEL_LAMBERT
    if (NdotL > 0.0f)
    {
        // 블린퐁 반사광
        float3 Specular = BlinnPhong(N, L, V, SpecularPower);
        OutSpecular = Specular * LightRadiance;
    }
#endif
}

float RadialAttenuation(float3 ToLight, float Radius, float FalloffExponent, float bUseInverseSquared)
{
    float DistanceSqr = dot(ToLight, ToLight);
    
    float RadiusSqr = Radius * Radius;
    if (DistanceSqr >= RadiusSqr)
        return 0.0f;
    
    float Factor = DistanceSqr / RadiusSqr;
    if (bUseInverseSquared > 0.5f)
    {
        // 물리 기반 역제곱 감쇠
        float Falloff = saturate(1.0f - Factor * Factor);
        return (Falloff * Falloff) / (DistanceSqr + 1.0f);
    }
    else
    {
        // 비물리 지수 감쇠: LightFalloffExponent 사용
        return pow(saturate(1.0f - Factor), max(FalloffExponent, 0.001f));
    }
}

float SpotAttenuation(float3 L, float3 SpotDirection, float2 SpotAngles)
{
    float cosTheta = dot(SpotDirection, -L);
    
    float Falloff = saturate(cosTheta * SpotAngles.x + SpotAngles.y);

    return Falloff * Falloff;
}

void CalculateLighting(float3 N, float3 V, float3 WorldPos, out float3 OutDiffuse, out float3 OutSpecular)
{
    // 산란광과 반사광 초기화
    float3 TotalDiffuse = float3(0, 0, 0);
    float3 TotalSpecular = float3(0, 0, 0);
    float MatSpecularPower = max(SpecularPower, 1.0f);
    
    // Directional Light 계산
    if (DirectionalLightCount > 0)
    {
        // Directional Light의 방향은 빛이 향하는 방향이므로, 표면에 도달하는 빛의 방향은 반대이다.
        float3 DirectionL = normalize(-Directional.Direction.xyz);
        // Directional Light의 세기는 ColorIntensity의 RGB에 Alpha(Intensity)를 곱한 값이다.
        float3 DirectionalRadiance = Directional.ColorIntensity.rgb * Directional.ColorIntensity.a;
        
        float3 DirectionalDiffuse = float3(0, 0, 0);
        float3 DirectionalSpecular = float3(0, 0, 0);
        
        DefaultLit(N, V, DirectionL, DirectionalRadiance, MatSpecularPower, DirectionalDiffuse, DirectionalSpecular);
        
        TotalDiffuse += DirectionalDiffuse;
        TotalSpecular += DirectionalSpecular;
    }
    
    for (int i = 0; i < PointLightCount; i++)
    {
        float3 PointLightPos = PointLights[i].PositionRadius.xyz;
        float PointLightRadius = PointLights[i].PositionRadius.w;
        float PointFalloffExp = PointLights[i].FalloffParams.x;
        float PointUseInvSq = PointLights[i].FalloffParams.y;
        
        float3 ToLight = PointLightPos - WorldPos;
        float Attenuation = RadialAttenuation(ToLight, PointLightRadius, PointFalloffExp, PointUseInvSq);
        
        // 반경 안에 들어와 감쇠가 0보다 클 때만 정규화(normalize) 및 조명 계산 수행
        if (Attenuation > 0.0f)
        {
            float3 L = normalize(ToLight);
            float3 PointLightRadiance = (PointLights[i].ColorIntensity.rgb * PointLights[i].ColorIntensity.a) * Attenuation;
            
            float3 PointLightDiffuse = float3(0, 0, 0);
            float3 PointLightSpecular = float3(0, 0, 0);
            DefaultLit(N, V, L, PointLightRadiance, MatSpecularPower, PointLightDiffuse, PointLightSpecular);
        
            TotalDiffuse += PointLightDiffuse;
            TotalSpecular += PointLightSpecular;
        }
    }
    
    for (int j = 0; j < SpotLightCount; j++)
    {
        float3 SpotLightPos = SpotLights[j].PositionRadius.xyz;
        float SpotLightRadius = SpotLights[j].PositionRadius.w;
        float2 SpotAngles = SpotLights[j].ConeScaleOffset.xy;
        float SpotFalloffExp = SpotLights[j].ConeScaleOffset.z;
        float SpotUseInvSq = SpotLights[j].ConeScaleOffset.w;
        
        float3 ToLight = SpotLightPos - WorldPos;
        float DistanceAttenuation = RadialAttenuation(ToLight, SpotLightRadius, SpotFalloffExp, SpotUseInvSq);
        
        // 거리 반경 안에 들어왔을 때만 방향 정규화 및 원뿔 감쇠 계산 수행
        if (DistanceAttenuation > 0.0f)
        {
            float3 L = normalize(ToLight);
            float3 SpotDirection = normalize(SpotLights[j].DirectionalAngle.xyz);
            float ConeAttenuation = SpotAttenuation(L, SpotDirection, SpotAngles);
            float Attenuation = DistanceAttenuation * ConeAttenuation;
            
            if (Attenuation > 0.0f)
            {
                float3 SpotLightRadiance = (SpotLights[j].ColorIntensity.rgb * SpotLights[j].ColorIntensity.a) * Attenuation;
                
                float3 SpotLightDiffuse = float3(0, 0, 0);
                float3 SpotLightSpecular = float3(0, 0, 0);
                DefaultLit(N, V, L, SpotLightRadiance, MatSpecularPower, SpotLightDiffuse, SpotLightSpecular);
            
                TotalDiffuse += SpotLightDiffuse;
                TotalSpecular += SpotLightSpecular;
            }
        }
    }
    
    OutDiffuse = TotalDiffuse;
#if LIGHTING_MODEL_LAMBERT
    OutSpecular = float3(0, 0, 0);
#else
    OutSpecular = TotalSpecular;
#endif
}


void CalculateTileLighting(float3 N, float3 V, float3 WorldPos, uint TileIndex, out float3 OutDiffuse, out float3 OutSpecular)
{
    // 산란광과 반사광 초기화
    float3 TotalDiffuse = float3(0, 0, 0);
    float3 TotalSpecular = float3(0, 0, 0);
    float MatSpecularPower = max(SpecularPower, 1.0f);
    
    // Directional Light 계산
    if (DirectionalLightCount > 0)
    {
        // Directional Light의 방향은 빛이 향하는 방향이므로, 표면에 도달하는 빛의 방향은 반대이다.
        float3 DirectionL = normalize(-Directional.Direction.xyz);
        // Directional Light의 세기는 ColorIntensity의 RGB에 Alpha(Intensity)를 곱한 값이다.
        float3 DirectionalRadiance = Directional.ColorIntensity.rgb * Directional.ColorIntensity.a;
        
        float3 DirectionalDiffuse = float3(0, 0, 0);
        float3 DirectionalSpecular = float3(0, 0, 0);
        
        DefaultLit(N, V, DirectionL, DirectionalRadiance, MatSpecularPower, DirectionalDiffuse, DirectionalSpecular);
        
        TotalDiffuse += DirectionalDiffuse;
        TotalSpecular += DirectionalSpecular;
    }
    
    FTileLightRange pointRange = TilePointLightRanges[TileIndex];
    for (int i = 0; i < pointRange.Count; i++)
    {
        uint lightIndex = TilePointLightIndices[pointRange.Offset + i];
        float3 PointLightPos = PointLights[lightIndex].PositionRadius.xyz;
        float PointLightRadius = PointLights[lightIndex].PositionRadius.w;
        float PointFalloffExp = PointLights[lightIndex].FalloffParams.x;
        float PointUseInvSq = PointLights[lightIndex].FalloffParams.y;
        
        float3 ToLight = PointLightPos - WorldPos;
        float Attenuation = RadialAttenuation(ToLight, PointLightRadius, PointFalloffExp, PointUseInvSq);
        
        // 반경 안에 들어와 감쇠가 0보다 클 때만 정규화(normalize) 및 조명 계산 수행
        if (Attenuation > 0.0f)
        {
            float3 L = normalize(ToLight);
            float3 PointLightRadiance = (PointLights[lightIndex].ColorIntensity.rgb * PointLights[lightIndex].ColorIntensity.a) * Attenuation;
            
            float3 PointLightDiffuse = float3(0, 0, 0);
            float3 PointLightSpecular = float3(0, 0, 0);
            DefaultLit(N, V, L, PointLightRadiance, MatSpecularPower, PointLightDiffuse, PointLightSpecular);
        
            TotalDiffuse += PointLightDiffuse;
            TotalSpecular += PointLightSpecular;
        }
    }
    
    FTileLightRange spotRange = TileSpotLightRanges[TileIndex];
    for (int j = 0; j < spotRange.Count; j++)
    {
        uint lightIndex = TileSpotLightIndices[spotRange.Offset + j];
        float3 SpotLightPos = SpotLights[lightIndex].PositionRadius.xyz;
        float SpotLightRadius = SpotLights[lightIndex].PositionRadius.w;
        float2 SpotAngles = SpotLights[lightIndex].ConeScaleOffset.xy;
        float SpotFalloffExp = SpotLights[lightIndex].ConeScaleOffset.z;
        float SpotUseInvSq = SpotLights[lightIndex].ConeScaleOffset.w;
        
        float3 ToLight = SpotLightPos - WorldPos;
        float DistanceAttenuation = RadialAttenuation(ToLight, SpotLightRadius, SpotFalloffExp, SpotUseInvSq);
        
        // 거리 반경 안에 들어왔을 때만 방향 정규화 및 원뿔 감쇠 계산 수행
        if (DistanceAttenuation > 0.0f)
        {
            float3 L = normalize(ToLight);
            float3 SpotDirection = normalize(SpotLights[lightIndex].DirectionalAngle.xyz);
            float ConeAttenuation = SpotAttenuation(L, SpotDirection, SpotAngles);
            float Attenuation = DistanceAttenuation * ConeAttenuation;
            
            if (Attenuation > 0.0f)
            {
                float3 SpotLightRadiance = (SpotLights[lightIndex].ColorIntensity.rgb * SpotLights[lightIndex].ColorIntensity.a) * Attenuation;
                
                float3 SpotLightDiffuse = float3(0, 0, 0);
                float3 SpotLightSpecular = float3(0, 0, 0);
                DefaultLit(N, V, L, SpotLightRadiance, MatSpecularPower, SpotLightDiffuse, SpotLightSpecular);
            
                TotalDiffuse += SpotLightDiffuse;
                TotalSpecular += SpotLightSpecular;
            }
        }
    }
    
    OutDiffuse = TotalDiffuse;
#if LIGHTING_MODEL_LAMBERT
    OutSpecular = float3(0, 0, 0);
#else
    OutSpecular = TotalSpecular;
#endif
}

// 밴딩 방지용 인터리브드 그레이디언트 노이즈 (0.0 ~ 1.0 반환)
float InterleavedGradientNoise(float2 pixelPos)
{
    const float3 Magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(Magic.z * frac(dot(pixelPos, Magic.xy)));
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    float3 worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(float4(worldPos, 1.0f), VP);
    output.color = input.c;
    output.uv = input.t;

#if VIEW_MODE_NORMAL
    // w=0으로 이동 성분을 빼고 역전치 행렬로 월드 법선을 구한다.
    output.worldNormal = normalize(mul(float4(input.n, 0.0f), WorldInverseTranspose).xyz);
#elif LIGHTING_MODEL_LAMBERT || LIGHTING_MODEL_PHONG
    output.normal = mul(float4(input.n, 0.0f), WorldInverseTranspose).xyz;
    output.worldPos = worldPos;
#elif LIGHTING_MODEL_GOURAUD
    float3 worldNormal = mul(float4(input.n, 0.0f), WorldInverseTranspose).xyz;
    CalculateLighting(
        normalize(worldNormal),
        normalize(CameraPosition.xyz - worldPos),
        worldPos,
        output.diffuseLighting,
        output.specularLighting);
#endif

    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
#if VIEW_MODE_NORMAL
    float3 normalColor = normalize(input.worldNormal) * 0.5f + 0.5f;
    return float4(normalColor, 1.0f);
#elif VIEW_MODE_LIGHT_CULLING
    return VisualizeTileLightCount(input.position.xy);
#else
    float2 uv = input.uv + UVOffset;
    float4 ambientTex = g_txAmbient.Sample(g_Sample, uv);
    float4 diffuseTex = g_txDiffuse.Sample(g_Sample, uv);
    float4 specularTex = g_txSpecular.Sample(g_Sample, uv);

    // 감마 보정 (sRGB -> 선형)
    ambientTex.rgb = pow(max(ambientTex.rgb, 0.0f), 2.2f);
    diffuseTex.rgb = pow(max(diffuseTex.rgb, 0.0f), 2.2f);

    float3 ambientSurface = ambientTex.rgb * Ka * BaseColor.rgb;
    float3 diffuseSurface = diffuseTex.rgb * Kd * BaseColor.rgb;
    float3 specularSurface = specularTex.rgb * Ks;

    float3 FinalRGB;

#if LIGHTING_MODEL_UNLIT
    FinalRGB = diffuseSurface;
#elif LIGHTING_MODEL_GOURAUD
    float3 AmbientLight = CalculateAmbientLight(Ambient);
    FinalRGB = AmbientLight * ambientSurface + input.diffuseLighting * diffuseSurface + input.specularLighting * specularSurface;
#elif LIGHTING_MODEL_LAMBERT || LIGHTING_MODEL_PHONG
    float3 N = normalize(input.normal);
    float3 V = normalize(CameraPosition.xyz - input.worldPos);
    
    float3 AmbientLight = CalculateAmbientLight(Ambient);
    float3 Diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 Specular = float3(0.0f, 0.0f, 0.0f);
    
    if (TileLightEnabled > 0)
    {
        uint TileIndex = GetTileIndex(input.position.xy);
        CalculateTileLighting(N, V, input.worldPos, TileIndex, Diffuse, Specular);
    }
    else
    {
        CalculateLighting(N, V, input.worldPos, Diffuse, Specular);
    }
    
    FinalRGB = AmbientLight * ambientSurface + Diffuse * diffuseSurface + Specular * specularSurface;
#else
    FinalRGB = diffuseSurface;
#endif
    
    // 감마 보정 (선형 -> sRGB)
    FinalRGB = pow(max(FinalRGB, 0.0f), 1.0f / 2.2f);
    // 디더링 생성
    float dither = InterleavedGradientNoise(input.position.xy);
    FinalRGB += (dither - 0.5f) / 255.0f;
    
    float FinalAlpha = bOpaque > 0.5f ? 1.0f : (diffuseTex.a * BaseColor.a);
    return float4(FinalRGB, FinalAlpha);
#endif
}