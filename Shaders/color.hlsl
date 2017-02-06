//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

struct VS_INPUT
{
	float3 pos : POSITION;
	float4 color: COLOR;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

cbuffer cb0 : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float deltaTime;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);
	output.color = input.color;
	return output;

}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return input.color;
}
