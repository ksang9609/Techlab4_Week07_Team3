#pragma pack_matrix(row_major)
#define LIGHTING_MODEL_GOURAUD 1
//#define LIGHTING_MODEL_LAMBERT 1
//#define LIGHTING_MODEL_PHONG 1
//#define LIGHTING_MODEL_UNLIT 1

cbuffer Viewconstants : register(b0)
{
    matrix VP;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float2 UVOffset;
    float bOpaque;
    float Padding;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
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

cbuffer CameraConstants: register(b4)
{
    float3 CameraWorldPosition;
    float CameraPadding;
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
    float3 vertexLighting : TEXCOORD2;
    float3 vertexSpecular : TEXCOORD3;
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

// 임시 하드코딩 Directional Light. 빛이 "향하는" 방향이다.
static const float3 LightDir = normalize(float3(0.5f, 0.5f, -1.0f));
static const float3 LightColor = float3(0.8f, 0.8f, 0.8f);
static const float3 AmbientColor = float3(0.2f, 0.2f, 0.2f);

// Gouraud와 Lambert의 공통 조명 계산 함수. 각각 VS와 PS에서 사용한다.
float3 CalculateDirectionalDiffuse(float3 Normal)
{
    float3 N = normalize(Normal);
    float NdotL = saturate(dot(N, -LightDir));
    
    return LightColor * NdotL;
}

float3 CalculateAmbientLight(float3 AmbientColor, float Intensity)
{
    return AmbientColor * Intensity;
}

// Blinn-Phong 정반사 계산. 법선과 Halfway Vector의 일치 정도로 반사광을 구한다.
float3 CalculateDirectionalSpecular(float3 Normal, float3 WorldPos, float3 CameraPos)
{
    float3 N = normalize(Normal);
    float3 L = normalize(-LightDir);
    float3 V = normalize(CameraPos - WorldPos);
    float3 H = normalize(L + V);
    
    float Shininess = 8.0f;
    float SpecularStrength = 1.0f;
    float Specular = pow(saturate(dot(N, H)), Shininess);
    
    // 빛을 받지 않는 뒷면에는 반사광을 적용하지 않는다.
    return LightColor * Specular * SpecularStrength * step(0.0001f, dot(N, L));
}

// CalculatePointLight()
// CalculateSpotLight()

// 기존 FireBall Point Light의 확산 반사 계산. VS와 PS에서 공통 사용한다.
float3 CalculateFireBallDiffuse(float3 Normal, float3 WorldPos)
{
    float3 N = normalize(Normal);
    float3 totalPointLight = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < FireBallCount; ++i)
    {
        FireBallLight fireBall = FireBalls[i];
        float4 PositionRadius = fireBall.PositionRadius;
        float4 ColorIntensity = fireBall.ColorIntensity;
        float4 FalloffEnabled = fireBall.FalloffEnabled;
        
        float distance = length(PositionRadius.xyz - WorldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float3 lightDir = normalize(PositionRadius.xyz - WorldPos);
        
        float pointNdotL = saturate(dot(N, lightDir));
        float3 pointLight = ColorIntensity.rgb * pointNdotL * falloff * enabled * intensity;
        totalPointLight += pointLight;
    }
    return totalPointLight;
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(mul(float4(input.p, 1.0f), World), VP);
    
    // w=0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(input.n, 0.0f), World).xyz;
    
    // Gouraud: 정점별 Diffuse 계산
#if defined(LIGHTING_MODEL_GOURAUD)
    // 정점마다 AmbientColor, Diffuse, Specular를 계산한 뒤 PS로 전달한다.
    output.vertexLighting = AmbientColor + CalculateDirectionalDiffuse(output.normal) + CalculateFireBallDiffuse(output.normal, output.worldPos);
    output.vertexSpecular = CalculateDirectionalSpecular(output.normal, output.worldPos, CameraWorldPosition);
#else
    output.vertexLighting = float3(0.0f, 0.0f, 0.0f);
    output.vertexSpecular = float3(0.0f, 0.0f, 0.0f);
#endif

    output.color = input.c;
    output.uv = input.t;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Mesh 표면색
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;
    
    // Opaque는 알파를 1로 고정한다.
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;
    
#if defined(LIGHTING_MODEL_UNLIT)
    // 조명 계산 없이 텍스처와 머터리얼의 표면색만 출력한다.
    return float4(albedo.rgb, alpha);
#else
    float3 N = normalize(input.normal); // 현재 표면이 향하고 있는 방향
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
#if defined(LIGHTING_MODEL_GOURAUD)
    // VS에서 계산하고 보간된 Diffuse와 Specular를 사용한다.
    lighting = input.vertexLighting;
    specular = input.vertexSpecular;
    
#elif defined(LIGHTING_MODEL_LAMBERT)
    // 픽셀마다 확산 반사를 계산한다.
    lighting = AmbientColor + CalculateDirectionalDiffuse(N) + CalculateFireBallDiffuse(N, input.worldPos);
    
#elif defined(LIGHTING_MODEL_PHONG)
    // 픽셀마다 확산 반사와 정반사를 계산한다.
    lighting = AmbientColor + CalculateDirectionalDiffuse(N) + CalculateFireBallDiffuse(N, input.worldPos);
    specular = CalculateDirectionalSpecular(N, input.worldPos, CameraWorldPosition);
    
#else
    #error Lighting model is not defined.
#endif
    
    float3 finalRGB = albedo.rgb * lighting + specular;
    return float4(finalRGB, alpha);
#endif
}
