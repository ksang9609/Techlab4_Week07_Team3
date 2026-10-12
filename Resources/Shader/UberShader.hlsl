#pragma pack_matrix(row_major)

#define NUM_POINT_LIGHT 16
#define NUM_SPOT_LIGHT 16

cbuffer Viewconstants : register(b0)
{
    matrix VP;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
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
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

float3 CalculateAmbientLight(FAmbientLightInfo info)
{
    // Ambient Light 계산
    float3 Result = info.ColorIntensity.rgb * info.ColorIntensity.a;
    return Result;
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
    else
    {
        OutSpecular = float3(0, 0, 0);
    }
#endif
}

float RadialAttenuation(float3 LightPos, float3 WorldPos, float Radius, float FalloffExponent, float bUseInverseSquared)
{
    float3 ToLight = LightPos - WorldPos;
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

float4 CalculateLighting(float3 N, float3 V, float3 WorldPos, float4 albedo, bool bOpaque)
{
    // 최종 색상 초기화 (알파 값은 1로 설정)
    float4 FinalColor = float4(0, 0, 0, 1);
    
    // 최종 색상 계산을 위한 산란광과 반사광 초기화
    float3 TotalDiffuse = float3(0, 0, 0);
    float3 TotalSpecular = float3(0, 0, 0);
    float MatSpecularPower = max(SpecularPower, 1.0f);
    
    // Ambient Light 계산
    float3 AmbientLight = CalculateAmbientLight(Ambient);
    
    // Directional Light 계산
    if (DirectionalLightCount > 0)
    {
        // Directional Light의 방향은 빛이 향하는 방향이므로, 표면에 도달하는 빛의 방향은 반대이다.
        float3 DirectionL = normalize(-Directional.Direction.xyz);
        // Directional Light의 세기는 ColorIntensity의 RGB에 Alpha(Intensity)를 곱한 값이다.
        float3 DirectionalRadiance = Directional.ColorIntensity.rgb * Directional.ColorIntensity.a;
        
        // 람베르트 산란광과 블린퐁 반사광 계산
        float3 DirectionalDiffuse = float3(0, 0, 0);
        float3 DirectionalSpecular = float3(0, 0, 0);
        
        DefaultLit(N, V, DirectionL, DirectionalRadiance, MatSpecularPower, DirectionalDiffuse, DirectionalSpecular);
        
        // 최종 색상에 Ambient Light, Directional Light의 산란광과 반사광을 합산
        TotalDiffuse += DirectionalDiffuse;
        TotalSpecular += DirectionalSpecular;
    }
    
    for (int i = 0; i < PointLightCount; i++)
    {
        // Point Light의 위치
        float3 PointLightPos = PointLights[i].PositionRadius.xyz;
        // Point Light의 세기 계산 (RGB * Intensity)
        float3 PointLightRadiance = PointLights[i].ColorIntensity.rgb * PointLights[i].ColorIntensity.a;
        // Point Light의 방향
        float3 L = normalize(PointLightPos - WorldPos);
        // Point Light의 반경
        float PointLightRadius = PointLights[i].PositionRadius.w;
        float PointFalloffExp = PointLights[i].FalloffParams.x;
        float PointUseInvSq = PointLights[i].FalloffParams.y;
        
        float3 PointLightDiffuse = float3(0, 0, 0);
        float3 PointLightSpecular = float3(0, 0, 0);
        
        // Point Light의 감쇠 계산
        float Attenuation = RadialAttenuation(PointLightPos, WorldPos, PointLightRadius, PointFalloffExp, PointUseInvSq);
        
        if (Attenuation > 0.0f)
        {
            PointLightRadiance *= Attenuation;
            // 람베르트 산란광과 블린퐁 반사광 계산
            DefaultLit(N, V, L, PointLightRadiance, MatSpecularPower, PointLightDiffuse, PointLightSpecular);
        
            // 최종 색상에 Point Light의 산란광과 반사광을 감쇠를 적용하여 합산
            TotalDiffuse += PointLightDiffuse;
            TotalSpecular += PointLightSpecular;
        }
    }
    
    for (int i = 0; i < SpotLightCount; i++)
    {
        // Spot Light의 위치
        float3 SpotLightPos = SpotLights[i].PositionRadius.xyz;
        // Spot Light의 세기 계산 (RGB * Intensity)
        float3 SpotLightRadiance = SpotLights[i].ColorIntensity.rgb * SpotLights[i].ColorIntensity.a;
        // Spot Light의 방향
        float3 SpotDirection = SpotLights[i].DirectionalAngle.xyz;
        // Spot Light의 각도
        float SpotLightAngle = SpotLights[i].DirectionalAngle.w;
        // Spot Light의 반경
        float SpotLightRadius = SpotLights[i].PositionRadius.w;
        // Spot Light와 픽셀의 빛 방향
        float3 L = normalize(SpotLightPos - WorldPos);
        // Spot Light의 콘 감쇠 스케일, 오프셋, 감쇠 파라미터
        float2 SpotAngles = SpotLights[i].ConeScaleOffset.xy;
        float SpotFalloffExp = SpotLights[i].ConeScaleOffset.z;
        float SpotUseInvSq = SpotLights[i].ConeScaleOffset.w;
        
        float3 SpotLightDiffuse = float3(0, 0, 0);
        float3 SpotLightSpecular = float3(0, 0, 0);
        
        // Spot Light의 감쇠 계산
        float DistanceAttenuation = RadialAttenuation(SpotLightPos, WorldPos, SpotLightRadius, SpotFalloffExp, SpotUseInvSq);
        float ConeAttenuation = SpotAttenuation(L, SpotDirection, SpotAngles);
        float Attenuation = DistanceAttenuation * ConeAttenuation;
        if (Attenuation > 0.0f)
        {
            SpotLightRadiance *= Attenuation;
            
            // 람베르트 산란광과 블린퐁 반사광 계산
            DefaultLit(N, V, L, SpotLightRadiance, MatSpecularPower, SpotLightDiffuse, SpotLightSpecular);
        
            // 최종 색상에 Spot Light의 산란광과 반사광을 감쇠를 적용하여 합산
            TotalDiffuse += SpotLightDiffuse;
            TotalSpecular += SpotLightSpecular;
        }
    }
    
#if LIGHTING_MODEL_LAMBERT
    // 최종 색상에 Ambient Light와 모든 조명의 산란광을 합산, 람베르트 모델이면 반사광은 무시
    FinalColor.rgb += (AmbientLight + TotalDiffuse) * albedo.rgb;
    FinalColor.a = bOpaque > 0.5f ? 1.0f : albedo.a; // 불투명 여부에 따라 알파 값 설정
#else
    // 최종 색상에 Ambient Light와 모든 조명의 산란광과 반사광을 합산
    FinalColor.rgb += (AmbientLight + TotalDiffuse) * albedo.rgb + TotalSpecular;
    FinalColor.a = bOpaque > 0.5f ? 1.0f : albedo.a; // 불투명 여부에 따라 알파 값 설정
#endif
    return FinalColor;
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(mul(float4(input.p, 1.0f), World), VP);
    
    // w=0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    //output.normal = mul(float4(input.n, 0.0f), World).xyz;
    output.normal = mul(float4(input.n, 0.0f), WorldInverseTranspose).xyz;
#if LIGHTING_MODEL_GOURAUD
    output.color = CalculateLighting(normalize(output.normal), normalize(CameraPosition.xyz - output.worldPos), output.worldPos, float4(1, 1, 1, 1), true);
#else
    output.color = input.c;
#endif
    output.uv = input.t;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Mesh 표면색
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    // BaseColor와 텍스처 색상을 곱하여 최종 알베도 색상 계산
    float4 albedo = texColor * BaseColor;
#if LIGHTING_MODEL_GOURAUD
    float4 FinalColor = input.color * albedo;
    FinalColor.a = bOpaque > 0.5f ? 1.0f : albedo.a;
    return FinalColor;
#elif LIGHTING_MODEL_UNLIT
    float4 FinalColor = albedo;
    FinalColor.a = bOpaque > 0.5f ? 1.0f : albedo.a;
    return FinalColor;
#else
    // 표면의 법선 벡터와 카메라 방향 벡터 계산
    float3 N = normalize(input.normal);
    float3 V = normalize(CameraPosition.xyz - input.worldPos);
    
    // 빛 계산을 통해 최종 색상 결정
    return CalculateLighting(N, V, input.worldPos, albedo, bOpaque);
#endif
}