//=============================================================================
// Lighting.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and lights geometry.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	PointLight gPointLight;
	DirectionalLight gDirLight;
	SpotLight gSpotLight;
	float3 gEyePosW;
	float gHeightScale;
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};


cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gViewProj;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	float4x4 gShadowTransform; 
	Material gMaterial;
};

Texture2D gShadowMap;
Texture2D gDiffuseMap;
Texture2D gNormalMap;
TextureCube gCubeMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter   = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComparisonFunc = LESS;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD0;
	float3 TangentW : TANGENT;
	float4 ShadowPosH : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	// Generate projective tex-coords to project shadow map onto scene.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	return vout;
}

//
// Tessellation
//

struct tess_VertexOut
{
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex     : TEXCOORD0;
	float  TessFactor : TESS;
};

tess_VertexOut tess_VS(VertexIn vin)
{
	tess_VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	// Tessellation factor
	float d = distance(vout.PosW, gEyePosW);

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate( (gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance) );
	
	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	vout.TessFactor = gMinTessFactor + tess*(gMaxTessFactor-gMinTessFactor);

	return vout;
}

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess  : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<tess_VertexOut,3> patch, 
                  uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess  = pt.EdgeTess[0];
	
	return pt;
}

struct HullOut
{
	float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<tess_VertexOut,3> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;
	hout.TangentW = p[i].TangentW;
	hout.Tex      = p[i].Tex;
	
	return hout;
}

// Hidden tessellation stage takes place

struct DomainOut
{
	float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float3 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(PatchTess patchTess, 
             float3 bary : SV_DomainLocation, 
             const OutputPatch<HullOut,3> tri)
{
	DomainOut dout;
	
	// Interpolate patch attributes to generated vertices.
	dout.PosW     = bary.x*tri[0].PosW     + bary.y*tri[1].PosW     + bary.z*tri[2].PosW;
	dout.NormalW  = bary.x*tri[0].NormalW  + bary.y*tri[1].NormalW  + bary.z*tri[2].NormalW;
	dout.TangentW = bary.x*tri[0].TangentW + bary.y*tri[1].TangentW + bary.z*tri[2].TangentW;
	dout.Tex      = bary.x*tri[0].Tex      + bary.y*tri[1].Tex      + bary.z*tri[2].Tex;
	
	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalW = normalize(dout.NormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.0f;
	float mipLevel = clamp( (distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
	
	// Sample height map (stored in alpha channel).
	float h = gNormalMap.SampleLevel(samLinear, dout.Tex, 0).a;
	
	// Offset vertex along normal.
	dout.PosW += (gHeightScale*(h))*dout.NormalW;

	// Generate projective tex-coords to project shadow map onto scene.
	dout.ShadowPosH = mul(float4(dout.PosW, 1.0f), gShadowTransform);
	
	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	
	return dout;
}
float4 tess_PS(DomainOut pin) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW); 

	// The toEye vector is used in lighting.
	float3 toEyeW = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEyeW);

	// Normalize.
	toEyeW /= distToEye;

	// Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);

	// Sample texture.
	texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex);

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

	//
	// Lighting.
	//

	float4 litColor = texColor;
	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Only the first light casts a shadow.
	float3 shadow = float3(1.0f, 1.0f, 1.0f);
	shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;  
	diffuse += shadow[0]*D;
	spec    += shadow[0]*S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;
	   
	litColor = texColor*(ambient + diffuse) + spec;

	// Common to take alpha from diffuse material.
	litColor.a = gMaterial.Diffuse.a * texColor.a;


	// Cube mapping
	float3 incident = -toEyeW;
	float3 reflectionVector = reflect(incident, bumpedNormalW);
	float4 reflectionColor  = gCubeMap.Sample(samAnisotropic, reflectionVector);

	litColor += 0.0f*reflectionColor;

    return litColor;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW); 

	// The toEye vector is used in lighting.
	float3 toEyeW = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEyeW);

	// Normalize.
	toEyeW /= distToEye;

	// Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);

	// Sample texture.
	texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex);

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

	//
	// Lighting.
	//

	float4 litColor = texColor;
	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Only the first light casts a shadow.
	float3 shadow = float3(1.0f, 1.0f, 1.0f);
	shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;  
	diffuse += shadow[0]*D;
	spec    += shadow[0]*S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;
	   
	litColor = texColor*(ambient + diffuse) + spec;

	// Common to take alpha from diffuse material.
	litColor.a = gMaterial.Diffuse.a * texColor.a;


	// Cube mapping
	float3 incident = -toEyeW;
	float3 reflectionVector = reflect(incident, bumpedNormalW);
	float4 reflectionColor  = gCubeMap.Sample(samAnisotropic, reflectionVector);

	litColor += 0.2f*reflectionColor;

    return litColor;
}

RasterizerState StateWireframe
{
	FillMode = Wireframe;
    CullMode = None;
};

technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 Tess1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, tess_VS() ) );
        SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, tess_PS() ) );

		//SetRasterizerState(StateWireframe);
    }
}



