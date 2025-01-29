// EnvironmentMap.fx
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float3 EyePos;
    float padding; // Padding to align to 16 bytes
};

TextureCube g_EnvironmentMap;
SamplerState g_Sampler;

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 WorldPos : TEXCOORD0;
    float3 ViewPos : TEXCOORD1; // Added ViewPos
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.Pos, 1.0f), World);
    output.WorldPos = worldPos.xyz;
    output.Normal = mul(input.Normal, (float3x3) World);
    float4 viewPos = mul(worldPos, View);
    output.ViewPos = viewPos.xyz; // Pass view space position to pixel shader
    output.Pos = mul(viewPos, Projection);
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    // Normalize the normal and view direction vectors
    float3 normal = normalize(input.Normal);
    float3 viewDir = normalize(input.ViewPos);

    // Calculate the reflection vector
    float3 reflected = reflect(-viewDir, normal);

    // Sample the environment map using the reflected vector
    float4 reflectionColor = g_EnvironmentMap.Sample(g_Sampler, reflected);

    // Base color (you can replace this with your object's base color)
    float4 baseColor = float4(1.0, 1.0, 1.0, 1.0); // White color for demonstration

    // Blend the reflection with the base color
    float reflectionIntensity = 0.5; // Adjust this value to control the shininess
    float4 finalColor = lerp(baseColor, reflectionColor, reflectionIntensity);

    return finalColor;
}
