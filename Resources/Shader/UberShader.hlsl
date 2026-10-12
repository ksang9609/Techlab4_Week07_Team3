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
#if LIGHTING_MODEL_GOURAUD
    float3 specular : TEXCOORD2;
#endif
};

Texture2D g_txColor : register(t0);
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

void CalculateLighting(float3 N, float3 V, float3 WorldPos, out float3 OutDiffuseAndAmbient, out float3 OutSpecular)
{
    // 산란광과 반사광 초기화
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
    
    for (int i = 0; i < SpotLightCount; i++)
    {
        float3 SpotLightPos = SpotLights[i].PositionRadius.xyz;
        float SpotLightRadius = SpotLights[i].PositionRadius.w;
        float2 SpotAngles = SpotLights[i].ConeScaleOffset.xy;
        float SpotFalloffExp = SpotLights[i].ConeScaleOffset.z;
        float SpotUseInvSq = SpotLights[i].ConeScaleOffset.w;
        
        float3 ToLight = SpotLightPos - WorldPos;
        float DistanceAttenuation = RadialAttenuation(ToLight, SpotLightRadius, SpotFalloffExp, SpotUseInvSq);
        
        // 거리 반경 안에 들어왔을 때만 방향 정규화 및 원뿔 감쇠 계산 수행
        if (DistanceAttenuation > 0.0f)
        {
            float3 L = normalize(ToLight);
            float3 SpotDirection = normalize(SpotLights[i].DirectionalAngle.xyz);
            float ConeAttenuation = SpotAttenuation(L, SpotDirection, SpotAngles);
            float Attenuation = DistanceAttenuation * ConeAttenuation;
            
            if (Attenuation > 0.0f)
            {
                float3 SpotLightRadiance = (SpotLights[i].ColorIntensity.rgb * SpotLights[i].ColorIntensity.a) * Attenuation;
                
                float3 SpotLightDiffuse = float3(0, 0, 0);
                float3 SpotLightSpecular = float3(0, 0, 0);
                DefaultLit(N, V, L, SpotLightRadiance, MatSpecularPower, SpotLightDiffuse, SpotLightSpecular);
            
                TotalDiffuse += SpotLightDiffuse;
                TotalSpecular += SpotLightSpecular;
            }
        }
    }
    
    OutDiffuseAndAmbient = AmbientLight + TotalDiffuse;
#if LIGHTING_MODEL_LAMBERT
    OutSpecular = float3(0, 0, 0);
#else
    OutSpecular = TotalSpecular;
#endif
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(float4(output.worldPos, 1.0f), VP);
    output.uv = input.t;

#if LIGHTING_MODEL_UNLIT
    output.normal = float3(0.0f, 0.0f, 0.0f);
    output.color = input.c;
#else
    // w=0으로 이동 성분을 빼고 역전치 행렬로 월드 법선을 구한다.
    output.normal = mul(float4(input.n, 0.0f), WorldInverseTranspose).xyz;
#if LIGHTING_MODEL_GOURAUD
    float3 DiffuseAndAmbient = float3(0.0f, 0.0f, 0.0f);
    float3 Specular = float3(0.0f, 0.0f, 0.0f);
    CalculateLighting(normalize(output.normal), normalize(CameraPosition.xyz - output.worldPos), output.worldPos, DiffuseAndAmbient, Specular);
    output.color = float4(DiffuseAndAmbient, 1.0f);
    output.specular = Specular;
#else
    output.color = input.c;
#endif
#endif

    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Mesh 표면색 (텍스처 * BaseColor)
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    // 감마 보정 (sRGB -> 선형)
    texColor = pow(texColor, 2.2f);
    float4 albedo = texColor * BaseColor;
    float3 FinalRGB;

#if LIGHTING_MODEL_UNLIT
    FinalRGB = albedo.rgb;
#elif LIGHTING_MODEL_GOURAUD
    FinalRGB = input.color.rgb * albedo.rgb + input.specular;
#else
    float3 N = normalize(input.normal);
    float3 V = normalize(CameraPosition.xyz - input.worldPos);
    
    float3 DiffuseAndAmbient = float3(0.0f, 0.0f, 0.0f);
    float3 Specular = float3(0.0f, 0.0f, 0.0f);
    CalculateLighting(N, V, input.worldPos, DiffuseAndAmbient, Specular);
    
    FinalRGB = DiffuseAndAmbient * albedo.rgb + Specular;
#endif
    
    // 감마 보정 (선형 -> sRGB)
    FinalRGB = pow(FinalRGB, 1.0f / 2.2f);
    // 디더링 생성
    float dither = frac(52.9829189f * frac(dot(input.position.xy, float2(0.06711056f, 0.00583715f))));
    FinalRGB += (dither - 0.5f) / 255.0f;
    
    float FinalAlpha = bOpaque > 0.5f ? 1.0f : albedo.a;
    return float4(FinalRGB, FinalAlpha);
}