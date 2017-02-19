//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 mTexCoord: TEXCOORD0;
	float3 normal : normal;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 mTexCoord : TEXCOORD;
};

cbuffer cb0 : register(b0)
{
	float4x4 view;
	float4x4 proj;
};

cbuffer cb1 : register(b1)
{
	float4x4 world;
	uint index;
};

Texture2D gDiffuseMap[6] : register(t0);

SamplerState gsamLinearWrap : register(s0);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);
	output.pos = mul(output.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);
	
	output.mTexCoord = input.mTexCoord;
	
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 albedo = gDiffuseMap[index].Sample(gsamLinearWrap, input.mTexCoord);
	return albedo;
}
