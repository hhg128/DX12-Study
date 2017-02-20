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
	float3 normal : normal;
};

cbuffer cb0 : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float3 lightDirection;
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

	// WVP transform
	output.pos = float4(input.pos, 1.0f);
	output.pos = mul(output.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);
	

	// Vertex Normal
	output.normal = mul(input.normal, world);
	output.normal = normalize(output.normal);
	
	
	// Texture UV
	output.mTexCoord = input.mTexCoord;
	
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 textureColor;
	float4 color;
	float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 ambientColor = float4(0.15f, 0.15f, 0.15f, 1.0f);
	float3 lightDir;
	float lightIntensity;

	textureColor = gDiffuseMap[index].Sample(gsamLinearWrap, input.mTexCoord);

	color = ambientColor;
	lightDir = -lightDirection;

	lightIntensity = saturate(dot(input.normal, lightDir));
	if (lightIntensity > 0.0f)
	{
		color += (diffuseColor * lightIntensity);
	}

	color = saturate(color);
	color = color * textureColor;

	return textureColor;
}
