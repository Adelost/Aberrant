#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "Util.h"
#include "Effects.h"
#include "RenderStates.h"

namespace Vertex
{
	struct posNormTex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	struct posNormTexTan
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 TangentU;
	};

	struct posTexBondsY
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMFLOAT2 BoundsY;
	};
}

class ShaderManager
{
private:
	ShaderManager()
	{
		layout_pos = 0;
		layout_posNormTex = 0;
		layout_posNormTexTan = 0;
		layout_posTexBoundY = 0;
	}
	void createLayout(ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC *desc_inputElement, UINT numElements, ID3D11InputLayout **layout, ID3DX11EffectTechnique *technique)
	{
		// Create the input layout
		D3DX11_PASS_DESC desc_pass;
		technique->GetPassByIndex(0)->GetDesc(&desc_pass);
		HR(device->CreateInputLayout(desc_inputElement, numElements, desc_pass.pIAInputSignature, 
			desc_pass.IAInputSignatureSize, layout));
	}

public:
	Effects effects;
	RenderStates states;
	ID3D11InputLayout* layout_pos;
	ID3D11InputLayout* layout_posNormTex;
	ID3D11InputLayout* layout_posNormTexTan;
	ID3D11InputLayout* layout_posTexBoundY;

	static ShaderManager* getInstance()
	{
		static ShaderManager instance;
		return &instance;
	};
	~ShaderManager()
	{
		ReleaseCOM(layout_pos);
		ReleaseCOM(layout_posNormTex);
		ReleaseCOM(layout_posNormTexTan);
		ReleaseCOM(layout_posTexBoundY);

		effects.~Effects();
		states.~RenderStates();
	}
	
	void init(ID3D11Device* device)
	{
		effects.init(device);
		states.init(device);

		//
		// Create input layouts
		//

		D3D11_INPUT_ELEMENT_DESC desc_pos[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		createLayout(device, desc_pos, 1, &layout_pos, effects.fx_skybox->SkyTech);

		D3D11_INPUT_ELEMENT_DESC desc_posNormTex[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		createLayout(device, desc_posNormTex, 3, &layout_posNormTex, effects.fx_buildShadowMap->BuildShadowMapTech);

		D3D11_INPUT_ELEMENT_DESC desc_posNormTexTan[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		createLayout(device, desc_posNormTexTan, 4, &layout_posNormTexTan, effects.fx_standard->tech_light1);

		D3D11_INPUT_ELEMENT_DESC desc_posTexBoundY[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		createLayout(device, desc_posTexBoundY, 3, &layout_posTexBoundY, effects.fx_standard->tech_terrain);
	}
};

#endif