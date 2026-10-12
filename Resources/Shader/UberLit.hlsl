#pragma pack_matrix(row_major)

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

struct PointLight
{
    float4 PositionRadius; // xyz: 위치, w: 반경
    float4 ColorIntensity; // rgb: 색상, w: 강도
    float4 FalloffEnabled; // x: 감쇠 지수, y: 활성 여부
};

struct SpotLight
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
    float4 DirectionOuterConeCos;
};

cbuffer ScreenLightConstants : register(b5)
{
    float4 AmbientColorIntensity; // rgb: 색상, w: 강도
    float4 DirectionalColorIntensity; // rgb: 색상, w: 강도
    float4 DirectionalDirection; // xyz: 빛이 향하는 방향
    
    PointLight PointLights[4];
    uint PointLightCount;
    float3 PointLightPadding;
    
    SpotLight SpotLights[4];
    uint SpotLightCount;
    float3 SpotLightPadding;
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

// Gouraud와 Lambert의 공통 조명 계산 함수. 각각 VS와 PS에서 사용한다.
float3 CalculateDirectionalDiffuse(float3 Normal)
{
    float3 N = normalize(Normal);
    float3 L = -normalize(DirectionalDirection.xyz);
    float NdotL = saturate(dot(N, L));
    
    return DirectionalColorIntensity.rgb * DirectionalColorIntensity.w * NdotL;
}

float3 CalculateAmbientLight(float3 AmbientColor, float Intensity)
{
    return AmbientColor * Intensity;
}

// Blinn-Phong 정반사 계산. 법선과 Halfway Vector의 일치 정도로 반사광을 구한다.
float3 CalculateDirectionalSpecular(float3 Normal, float3 WorldPos, float3 CameraPos)
{
    float3 N = normalize(Normal);
    float3 L = -normalize(DirectionalDirection.xyz);
    float3 V = normalize(CameraPos - WorldPos);
    float3 H = normalize(L + V);
    
    float Shininess = 8.0f;
    float SpecularStrength = 1.0f;
    float Specular = pow(saturate(dot(N, H)), Shininess);
    
    // 빛을 받지 않는 뒷면에는 반사광을 적용하지 않는다.
    return DirectionalColorIntensity.rgb * DirectionalColorIntensity.w * Specular * SpecularStrength * step(0.0001f, dot(N, L));
}

// 기존 FireBall의 확산 반사 계산. VS와 PS에서 공통 사용한다.
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

float3 CalculatePointDiffuse(float3 Normal, float3 WorldPos)
{
    float3 N = normalize(Normal);
    float3 totalPointLight = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < PointLightCount; ++i)
    {
        PointLight pointlight = PointLights[i];
        float4 PositionRadius = pointlight.PositionRadius;
        float4 ColorIntensity = pointlight.ColorIntensity;
        float4 FalloffEnabled = pointlight.FalloffEnabled;
        
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

float3 CalculatePointSpecular(float3 Normal, float3 WorldPos, float3 CameraPos)
{
    float3 N = normalize(Normal);
    float3 V = normalize(CameraPos - WorldPos);
    float3 TotalSpecular = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < PointLightCount; ++i)
    {
        PointLight Light = PointLights[i];

        // 여기서 광원별 Specular 계산
        float3 L = normalize(Light.PositionRadius.xyz - WorldPos);
        float3 H = normalize(L + V);
        
        float Shininess = 8.0f;
        float SpecularStrength = 1.0f;
        float Specular = pow(saturate(dot(N, H)), Shininess);
        
        float distance = length(Light.PositionRadius.xyz - WorldPos);
        float radius = max(Light.PositionRadius.w, 0.001f);
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(Light.FalloffEnabled.x, 0.001f));
        float enabled = Light.FalloffEnabled.y;
        
        TotalSpecular += Light.ColorIntensity.rgb * Light.ColorIntensity.w * Specular * SpecularStrength * step(0.0001f, dot(N, L)) * falloff * enabled;
    }

    return TotalSpecular;
}

float3 CalculateSpotDiffuse(float3 Normal, float3 WorldPos)
{
    float3 N = normalize(Normal);
    float3 totalSpotLight = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < SpotLightCount; ++i)
    {
        SpotLight spotlight = SpotLights[i];
        float4 PositionRadius = spotlight.PositionRadius;
        float4 ColorIntensity = spotlight.ColorIntensity;
        float4 FalloffEnabled = spotlight.FalloffEnabled;
        float4 DirectionOuterConeCos = spotlight.DirectionOuterConeCos;
        
        float distance = length(PositionRadius.xyz - WorldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float3 lightDir = normalize(PositionRadius.xyz - WorldPos);
        
        float spotNdotL = saturate(dot(N, lightDir));
        float3 spotLight = ColorIntensity.rgb * spotNdotL * falloff * enabled * intensity;
        
        float3 LightToSurface = normalize(WorldPos - PositionRadius.xyz);
        float3 LightForward = normalize(DirectionOuterConeCos.xyz);
        float ConeCos = dot(LightForward, LightToSurface);

        float InnerCos = FalloffEnabled.z;
        float OuterCos = DirectionOuterConeCos.w;

        float ConeAttenuation = saturate((ConeCos - OuterCos) / max(InnerCos - OuterCos, 0.0001f));
        
        totalSpotLight += spotLight * ConeAttenuation;
    }
    return totalSpotLight;
}

float3 CalculateSpotSpecular(float3 Normal, float3 WorldPos, float3 CameraPos)
{
    float3 N = normalize(Normal);
    float3 V = normalize(CameraPos - WorldPos);
    float3 TotalSpecular = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < SpotLightCount; ++i)
    {
        SpotLight Light = SpotLights[i];

        // 여기서 광원별 Specular 계산
        float3 L = normalize(Light.PositionRadius.xyz - WorldPos);
        float3 H = normalize(L + V);
        
        float Shininess = 8.0f;
        float SpecularStrength = 1.0f;
        float Specular = pow(saturate(dot(N, H)), Shininess);
        
        float distance = length(Light.PositionRadius.xyz - WorldPos);
        float radius = max(Light.PositionRadius.w, 0.001f);
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(Light.FalloffEnabled.x, 0.001f));
        float enabled = Light.FalloffEnabled.y;
        
        float3 LightToSurface = normalize(WorldPos - Light.PositionRadius.xyz);
        float3 LightForward = normalize(Light.DirectionOuterConeCos.xyz);
        float ConeCos = dot(LightForward, LightToSurface);

        float InnerCos = Light.FalloffEnabled.z;
        float OuterCos = Light.DirectionOuterConeCos.w;
        float ConeAttenuation = saturate((ConeCos - OuterCos) / max(InnerCos - OuterCos, 0.0001f));
        
        TotalSpecular += Light.ColorIntensity.rgb * Light.ColorIntensity.w * Specular * SpecularStrength * step(0.0001f, dot(N, L)) * falloff * enabled * ConeAttenuation;
    }

    return TotalSpecular;
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
    output.vertexLighting = CalculateAmbientLight(AmbientColorIntensity.rgb, AmbientColorIntensity.w) + CalculateDirectionalDiffuse(output.normal) + CalculateFireBallDiffuse(output.normal, output.worldPos) + CalculatePointDiffuse(output.normal, output.worldPos) + CalculateSpotDiffuse(output.normal, output.worldPos);
    output.vertexSpecular = CalculateDirectionalSpecular(output.normal, output.worldPos, CameraWorldPosition) + CalculatePointSpecular(output.normal, output.worldPos, CameraWorldPosition) + CalculateSpotSpecular(output.normal, output.worldPos, CameraWorldPosition);
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
    lighting = CalculateAmbientLight(AmbientColorIntensity.rgb, AmbientColorIntensity.w) + CalculateDirectionalDiffuse(N) + CalculateFireBallDiffuse(N, input.worldPos) + CalculatePointDiffuse(N, input.worldPos) + CalculateSpotDiffuse(N, input.worldPos);
    
#elif defined(LIGHTING_MODEL_PHONG)
    // 픽셀마다 확산 반사와 정반사를 계산한다.
    lighting = CalculateAmbientLight(AmbientColorIntensity.rgb, AmbientColorIntensity.w) + CalculateDirectionalDiffuse(N) + CalculateFireBallDiffuse(N, input.worldPos) + CalculatePointDiffuse(N, input.worldPos) + CalculateSpotDiffuse(N, input.worldPos);
    specular = CalculateDirectionalSpecular(N, input.worldPos, CameraWorldPosition) + CalculatePointSpecular(N, input.worldPos, CameraWorldPosition) + CalculateSpotSpecular(N, input.worldPos, CameraWorldPosition);
    
#else
    #error Lighting model is not defined.
#endif
    
    float3 finalRGB = albedo.rgb * lighting + specular;
    return float4(finalRGB, alpha);
#endif
}
