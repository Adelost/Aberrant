#ifndef EFFECTS_H
#define EFFECTS_H

#include "Util.h"
#include "d3dx11Effect.h"
//
// Base class
//

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename)
	{
		fx = 0;

		// Create effect from the 'compiled' fx-file
		std::ifstream f(filename, std::ios::binary);
		f.seekg(0, std::ios_base::end);
		int size = (int)f.tellg();
		f.seekg(0, std::ios_base::beg);
		std::vector<char> compiledShader(size);
		f.read(&compiledShader[0], size);
		f.close();
		HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &fx));
	}
	virtual ~Effect()
	{
		ReleaseCOM(fx);
	}

protected:
	ID3DX11Effect* fx;
};
#pragma endregion


//
// Derived classes
//

#pragma region FXStandard
class FXStandard : public Effect
{
public:
	// Tech
	ID3DX11EffectTechnique* tech_light1;

	// Per object
	ID3DX11EffectMatrixVariable* world;
	void SetWorld(CXMMATRIX M)
	{
		world->SetMatrix(reinterpret_cast<const float*>(&M));
		XMMATRIX worldInvTranspose = Util::InverseTranspose(M);
		SetWorldInvTranspose(worldInvTranspose);
	}
	ID3DX11EffectMatrixVariable* worldInvTranspose;
	void SetWorldInvTranspose(CXMMATRIX M)				{worldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	ID3DX11EffectMatrixVariable* worldViewProj;
	void SetWorldViewProj(CXMMATRIX M)					{worldViewProj->SetMatrix(reinterpret_cast<const float*>(&M));}
	ID3DX11EffectVariable* fx_material;
	void SetMaterial(const Material& material)			{fx_material->SetRawValue(&material, 0, sizeof(Material));}
	ID3DX11EffectMatrixVariable* texTransform;
	void SetTexTransform(CXMMATRIX M)                   { texTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	ID3DX11EffectMatrixVariable* shadowTransform;
	void SetShadowTransform(CXMMATRIX M)                { shadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }

	// Per frame
	ID3DX11EffectVectorVariable* eyePosW;
	void SetEyePosW(const XMFLOAT3& v)					{eyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3));}
	ID3DX11EffectVariable* fx_dirLights;
	void SetDirLights(const DirectionalLight* lights)	{fx_dirLights->SetRawValue(lights, 0, sizeof(DirectionalLight));}
	ID3DX11EffectVariable* fx_pointLights; 
	void SetPointLights(const PointLight* lights)		{fx_pointLights->SetRawValue(lights, 0, sizeof(PointLight));}
	ID3DX11EffectVariable* fx_spotLights;
	void SetSpotLights(const SpotLight* lights)			{fx_spotLights->SetRawValue(lights, 0, sizeof(SpotLight));}

	// Resources
	ID3DX11EffectShaderResourceVariable* diffuseMap;
	void SetDiffuseMap(ID3D11ShaderResourceView* tex)   {diffuseMap->SetResource(tex); }
	ID3DX11EffectShaderResourceVariable* shadowMap;
	void SetShadowMap(ID3D11ShaderResourceView* tex)    { shadowMap->SetResource(tex); }
	ID3DX11EffectShaderResourceVariable* cubeMap;
	void SetCubeMap(ID3D11ShaderResourceView* tex)      { cubeMap->SetResource(tex); }
	

	FXStandard(ID3D11Device* device, const std::wstring& filename) : Effect(device, filename)
	{
		// Tech
		tech_light1    = fx->GetTechniqueByName("Light1");

		// Per object
		worldViewProj = fx->GetVariableByName("gWorldViewProj")->AsMatrix();
		world = fx->GetVariableByName("gWorld")->AsMatrix();
		worldInvTranspose = fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
		fx_material = fx->GetVariableByName("gMaterial");
		texTransform      = fx->GetVariableByName("gTexTransform")->AsMatrix();
		shadowTransform   = fx->GetVariableByName("gShadowTransform")->AsMatrix();

		// Per frame
		eyePosW           = fx->GetVariableByName("gEyePosW")->AsVector();
		fx_dirLights      = fx->GetVariableByName("gDirLight");
		fx_pointLights    = fx->GetVariableByName("gPointLight");
		fx_spotLights     = fx->GetVariableByName("gSpotLight");
	
		// Resources
		diffuseMap        = fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
		shadowMap         = fx->GetVariableByName("gShadowMap")->AsShaderResource();
		cubeMap           = fx->GetVariableByName("gCubeMap")->AsShaderResource();
	};
	~FXStandard(){}
};
#pragma endregion

#pragma region FXBuildShadowMap
class FXBuildShadowMap : public Effect
{
public:
	void SetViewProj(CXMMATRIX M)                       { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M)                          { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M)              { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M)                   { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	void SetHeightScale(float f)                        { HeightScale->SetFloat(f); }
	void SetMaxTessDistance(float f)                    { MaxTessDistance->SetFloat(f); }
	void SetMinTessDistance(float f)                    { MinTessDistance->SetFloat(f); }
	void SetMinTessFactor(float f)                      { MinTessFactor->SetFloat(f); }
	void SetMaxTessFactor(float f)                      { MaxTessFactor->SetFloat(f); }

	void SetDiffuseMap(ID3D11ShaderResourceView* tex)   { DiffuseMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex)    { NormalMap->SetResource(tex); }

	ID3DX11EffectTechnique* BuildShadowMapTech;
	ID3DX11EffectTechnique* BuildShadowMapAlphaClipTech;
	ID3DX11EffectTechnique* TessBuildShadowMapTech;
	ID3DX11EffectTechnique* TessBuildShadowMapAlphaClipTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MinTessFactor;
	ID3DX11EffectScalarVariable* MaxTessFactor;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;

	FXBuildShadowMap(ID3D11Device* device, const std::wstring& filename) : Effect(device, filename)
	{
		BuildShadowMapTech           = fx->GetTechniqueByName("BuildShadowMapTech");
		BuildShadowMapAlphaClipTech  = fx->GetTechniqueByName("BuildShadowMapAlphaClipTech");

		TessBuildShadowMapTech           = fx->GetTechniqueByName("TessBuildShadowMapTech");
		TessBuildShadowMapAlphaClipTech  = fx->GetTechniqueByName("TessBuildShadowMapAlphaClipTech");

		ViewProj          = fx->GetVariableByName("gViewProj")->AsMatrix();
		WorldViewProj     = fx->GetVariableByName("gWorldViewProj")->AsMatrix();
		World             = fx->GetVariableByName("gWorld")->AsMatrix();
		WorldInvTranspose = fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
		TexTransform      = fx->GetVariableByName("gTexTransform")->AsMatrix();
		EyePosW           = fx->GetVariableByName("gEyePosW")->AsVector();
		HeightScale       = fx->GetVariableByName("gHeightScale")->AsScalar();
		MaxTessDistance   = fx->GetVariableByName("gMaxTessDistance")->AsScalar();
		MinTessDistance   = fx->GetVariableByName("gMinTessDistance")->AsScalar();
		MinTessFactor     = fx->GetVariableByName("gMinTessFactor")->AsScalar();
		MaxTessFactor     = fx->GetVariableByName("gMaxTessFactor")->AsScalar();
		DiffuseMap        = fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
		NormalMap         = fx->GetVariableByName("gNormalMap")->AsShaderResource();
	};
	~FXBuildShadowMap(){}
};
#pragma endregion

#pragma region FXSkybox
class FXSkybox : public Effect
{
public:
	FXSkybox(ID3D11Device* device, const std::wstring& filename) : Effect(device, filename)
	{
		SkyTech       = fx->GetTechniqueByName("SkyTech");
		WorldViewProj = fx->GetVariableByName("gWorldViewProj")->AsMatrix();
		CubeMap       = fx->GetVariableByName("gCubeMap")->AsShaderResource();
	};
	~FXSkybox(){}

	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetCubeMap(ID3D11ShaderResourceView* cubemap)  { CubeMap->SetResource(cubemap); }

	ID3DX11EffectTechnique* SkyTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectShaderResourceVariable* CubeMap;
};
#pragma endregion

#pragma region FXShowTexture
class FXShowTexture : public Effect
{
public:
	FXShowTexture(ID3D11Device* device, const std::wstring& filename) : Effect(device, filename)
	{
		ViewArgbTech  = fx->GetTechniqueByName("ViewArgbTech");
		ViewRedTech   = fx->GetTechniqueByName("ViewRedTech");
		ViewGreenTech = fx->GetTechniqueByName("ViewGreenTech");
		ViewBlueTech  = fx->GetTechniqueByName("ViewBlueTech");
		ViewAlphaTech = fx->GetTechniqueByName("ViewAlphaTech");

		WorldViewProj = fx->GetVariableByName("gWorldViewProj")->AsMatrix();
		Texture       = fx->GetVariableByName("gTexture")->AsShaderResource();
	};
	~FXShowTexture(){}

	void SetWorldViewProj(CXMMATRIX M)              { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexture(ID3D11ShaderResourceView* tex)  { Texture->SetResource(tex); }

	ID3DX11EffectTechnique* ViewArgbTech;
	ID3DX11EffectTechnique* ViewRedTech;
	ID3DX11EffectTechnique* ViewGreenTech;
	ID3DX11EffectTechnique* ViewBlueTech;
	ID3DX11EffectTechnique* ViewAlphaTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectShaderResourceVariable* Texture;
};
#pragma endregion

// Manager class
#pragma region Effects
class Effects
{
public:
	FXStandard* fx_standard;
	FXBuildShadowMap* fx_buildShadowMap;
	FXSkybox* fx_skybox;
	FXShowTexture* fx_showTexture;

	Effects()
	{
	}
	~Effects()
	{
		SafeDelete(fx_standard);
		SafeDelete(fx_buildShadowMap);
		SafeDelete(fx_skybox);
		SafeDelete(fx_showTexture);
	}
	void init(ID3D11Device* device)
	{
		fx_standard = new FXStandard(device, L"FX/Output/Basic.fxo");
		fx_buildShadowMap = new FXBuildShadowMap(device, L"FX/Output/BuildShadowMap.fxo");
		fx_skybox = new FXSkybox(device, L"FX/Output/Sky.fxo");
		fx_showTexture = new FXShowTexture(device, L"FX/Output/ShowTexture.fxo");
	}
};
#pragma endregion

#endif