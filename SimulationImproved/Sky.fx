//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

TextureCube txSkyColor : register(t1);
SamplerState txSkySampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
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
    float3 ViewDir : TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    float4 inPos = input.Pos;
    output.ViewDir = inPos.xyz;
    //View may need to be an eyeposition, like in the cpp
    inPos.xyz += eyePos.xyz;
    inPos = mul(inPos, View);
    inPos = mul(inPos, Projection);
    output.Pos = inPos;

    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return txSkyColor.Sample(txSkySampler, input.ViewDir);
}