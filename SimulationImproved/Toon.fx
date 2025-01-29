Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float3 LightDirection;
    float padding1;
    float3 EyePos;
    float padding2;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    float4 pos = float4(input.Pos, 1.0f);
    output.Pos = mul(pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(input.Norm, (float3x3)World);
    output.WorldPos = mul(pos, World).xyz;
    output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.Norm);
    float4 materialAmb = float4(0.6, 0.6, 0.6, 1.0);
    float4 materialDiff = float4(0.9, 1.0, 1.0, 1.0);
    float4 lightColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 lightDirection = normalize(LightDirection);

    float3 combinedLightDir = normalize(lightDirection + normalize(EyePos - input.WorldPos));
    float NdotL = max(dot(normal, combinedLightDir), 0.0f);
    float quantizedNdotL = floor(NdotL * 4.0f) / 4.0f;
    float3 color = materialAmb.rgb + quantizedNdotL * lightColor.rgb;

    float edgeThreshold = 0.5f;
    float3 viewDir = normalize(EyePos - input.WorldPos);
    float NdotV = dot(normal, viewDir);
    if (NdotV < edgeThreshold)
    {
        color = float3(0.0f, 0.0f, 0.0f);
    }

    float4 texColor = txDiffuse.Sample(samLinear, input.Tex);
    return float4(color, 1.0f) * texColor;
}
