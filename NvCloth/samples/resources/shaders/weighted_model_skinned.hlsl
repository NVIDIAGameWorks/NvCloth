#include "common_buffers.hlsl"
#include "lighting.hlsl"

SamplerState defaultSampler : register(s0);
Texture2D diffuseTexture : register(t0);
Texture2D bonesTexture : register(t1);

struct VS_INPUT
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	uint4 boneIndices : TEXCOORD1;
	float4 boneWeights : TEXCOORD2;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPos : POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
};

VS_OUTPUT VS(VS_INPUT iV)
{
	VS_OUTPUT oV;

	float4x4 boneMatrix0;
	boneMatrix0[0] = bonesTexture.Load(int3(0, iV.boneIndices.x, 0));
	boneMatrix0[1] = bonesTexture.Load(int3(1, iV.boneIndices.x, 0));
	boneMatrix0[2] = bonesTexture.Load(int3(2, iV.boneIndices.x, 0));
	boneMatrix0[3] = bonesTexture.Load(int3(3, iV.boneIndices.x, 0));
	float4x4 boneMatrix1;
	boneMatrix1[0] = bonesTexture.Load(int3(0, iV.boneIndices.y, 0));
	boneMatrix1[1] = bonesTexture.Load(int3(1, iV.boneIndices.y, 0));
	boneMatrix1[2] = bonesTexture.Load(int3(2, iV.boneIndices.y, 0));
	boneMatrix1[3] = bonesTexture.Load(int3(3, iV.boneIndices.y, 0));
	float4x4 boneMatrix2;
	boneMatrix2[0] = bonesTexture.Load(int3(0, iV.boneIndices.z, 0));
	boneMatrix2[1] = bonesTexture.Load(int3(1, iV.boneIndices.z, 0));
	boneMatrix2[2] = bonesTexture.Load(int3(2, iV.boneIndices.z, 0));
	boneMatrix2[3] = bonesTexture.Load(int3(3, iV.boneIndices.z, 0));
	float4x4 boneMatrix3;
	boneMatrix3[0] = bonesTexture.Load(int3(0, iV.boneIndices.w, 0));
	boneMatrix3[1] = bonesTexture.Load(int3(1, iV.boneIndices.w, 0));
	boneMatrix3[2] = bonesTexture.Load(int3(2, iV.boneIndices.w, 0));
	boneMatrix3[3] = bonesTexture.Load(int3(3, iV.boneIndices.w, 0));
	
	float4x4 offsetMatrix0;
	offsetMatrix0[0] = bonesTexture.Load(int3(0, iV.boneIndices.x + boneoffsetoffset, 0));
	offsetMatrix0[1] = bonesTexture.Load(int3(1, iV.boneIndices.x + boneoffsetoffset, 0));
	offsetMatrix0[2] = bonesTexture.Load(int3(2, iV.boneIndices.x + boneoffsetoffset, 0));
	offsetMatrix0[3] = bonesTexture.Load(int3(3, iV.boneIndices.x + boneoffsetoffset, 0));
	float4x4 offsetMatrix1;
	offsetMatrix1[0] = bonesTexture.Load(int3(0, iV.boneIndices.y + boneoffsetoffset, 0));
	offsetMatrix1[1] = bonesTexture.Load(int3(1, iV.boneIndices.y + boneoffsetoffset, 0));
	offsetMatrix1[2] = bonesTexture.Load(int3(2, iV.boneIndices.y + boneoffsetoffset, 0));
	offsetMatrix1[3] = bonesTexture.Load(int3(3, iV.boneIndices.y + boneoffsetoffset, 0));
	float4x4 offsetMatrix2;
	offsetMatrix2[0] = bonesTexture.Load(int3(0, iV.boneIndices.z + boneoffsetoffset, 0));
	offsetMatrix2[1] = bonesTexture.Load(int3(1, iV.boneIndices.z + boneoffsetoffset, 0));
	offsetMatrix2[2] = bonesTexture.Load(int3(2, iV.boneIndices.z + boneoffsetoffset, 0));
	offsetMatrix2[3] = bonesTexture.Load(int3(3, iV.boneIndices.z + boneoffsetoffset, 0));
	float4x4 offsetMatrix3;
	offsetMatrix3[0] = bonesTexture.Load(int3(0, iV.boneIndices.w + boneoffsetoffset, 0));
	offsetMatrix3[1] = bonesTexture.Load(int3(1, iV.boneIndices.w + boneoffsetoffset, 0));
	offsetMatrix3[2] = bonesTexture.Load(int3(2, iV.boneIndices.w + boneoffsetoffset, 0));
	offsetMatrix3[3] = bonesTexture.Load(int3(3, iV.boneIndices.w + boneoffsetoffset, 0));


	float3 skinnedPos0 = mul(mul(float4(iV.position, 1.0f), offsetMatrix0), boneMatrix0).xyz;
	float3 skinnedPos1 = mul(mul(float4(iV.position, 1.0f), offsetMatrix1), boneMatrix1).xyz;
	float3 skinnedPos2 = mul(mul(float4(iV.position, 1.0f), offsetMatrix2), boneMatrix2).xyz;
	float3 skinnedPos3 = mul(mul(float4(iV.position, 1.0f), offsetMatrix3), boneMatrix3).xyz;
	float3 skinnedPos =
		skinnedPos0 * iV.boneWeights.x +
		skinnedPos1 * iV.boneWeights.y +
		skinnedPos2 * iV.boneWeights.z +
		skinnedPos3 * iV.boneWeights.w;

	float4 worldSpacePos = mul(float4(skinnedPos, 1.0f), model);
	oV.position = mul(worldSpacePos, viewProjection);

	oV.worldPos = worldSpacePos;

	// normals
	float3 skinnedNorm0 = mul(mul(float4(iV.normal, 0.0f), offsetMatrix0), boneMatrix0).xyz;
	float3 skinnedNorm1 = mul(mul(float4(iV.normal, 0.0f), offsetMatrix1), boneMatrix1).xyz;
	float3 skinnedNorm2 = mul(mul(float4(iV.normal, 0.0f), offsetMatrix2), boneMatrix2).xyz;
	float3 skinnedNorm3 = mul(mul(float4(iV.normal, 0.0f), offsetMatrix3), boneMatrix3).xyz;
	float3 localNormal =
		skinnedNorm0 * iV.boneWeights.x +
		skinnedNorm1 * iV.boneWeights.y +
		skinnedNorm2 * iV.boneWeights.z +
		skinnedNorm3 * iV.boneWeights.w;
	float3 worldNormal = mul(float4(localNormal, 0.0f), model).xyz;
	oV.normal = worldNormal;

	oV.uv = iV.uv;

	return oV;
}

float4 PS(VS_OUTPUT iV) : SV_Target0
{
	float3 lightColor = CalcPixelLight(defaultColor.xyz, iV.worldPos.xyz, iV.normal);	
	return float4(lightColor, 1);
}