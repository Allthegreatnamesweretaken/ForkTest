Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float3 LightDirection;
    float3 LightPosition;
    vector eyePos;
}
   

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : TEXCOORD1;
    float4 ViewPos : TEXCOORD2; // Add ViewPos to PS_INPUT
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    float4 worldPos = mul(input.Pos, World);
    float4 viewPos = mul(worldPos, View);
    output.Pos = mul(viewPos, Projection);
    output.Norm = mul(input.Norm, (float3x3) World);
    output.Tex = input.Tex;
    output.ViewPos = viewPos; // Pass view space position to pixel shader
    return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    // Define ambient material color
    float4 materialAmb = float4(0.5, 0.5, 0.5, 1.0);

    // Define diffuse material color
    float4 materialDiff = float4(0.9, 0.7, 1.0, 1.0);

    // Define light color
    float4 lightCol = float4(1.0, 0.6, 0.8, 1.0);

    // Normalize the input normal vector
    float3 normal = normalize(input.Norm);

    // Normalize the light direction
    float3 lightDir = normalize(LightDirection);

    // Calculate the diffuse lighting factor using the dot product of the light direction and the normal
    float diff = max(0.0, dot(lightDir, normal));

    // Calculate the final color by combining ambient and diffuse components with the light color
    float4 finalColor = (materialAmb + diff * materialDiff) * lightCol;

    // Set the alpha component of the final color to 1 (fully opaque)
    finalColor.a = 1;

    // Sample the texture at the given texture coordinates
    float4 color1 = txDiffuse.Sample(samLinear, input.Tex);

    // Multiply the final color by the sampled texture color
    float4 color = color1 * finalColor;

    // Define fog parameters
    float fogStart = 30.0f; // Start distance of the fog
    float fogEnd = 60.0f; // End distance of the fog
    float4 fogColor = float4(0.5, 0.5, 0.5, 1.0); // Fog color

    // Calculate the distance from the camera to the pixel in view space
    float distance = length(input.ViewPos.xyz);

    // Calculate the fog factor
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));

    // Blend the pixel color with the fog color
    color.rgb = lerp(color.rgb, fogColor.rgb, fogFactor);

    // Return the final color
    return color;
}






