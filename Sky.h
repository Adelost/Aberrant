#ifndef SKY_H
#define SKY_H

#include "Util.h"

#include "GeometryFactory.h"
#include "Camera.h"
#include "ShaderManager.h"

class Sky
{
private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	float height_scale;
	float height_offset;

	ID3D11ShaderResourceView* mCubeMapSRV;

	UINT mIndexCount;
public:
	Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
	{
		HR(D3DX11CreateShaderResourceViewFromFile(device, cubemapFilename.c_str(), 0, 0, &mCubeMapSRV, 0));

		GeometryFactory::MeshData sphere;
		GeometryFactory geoGen;
		geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

		std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

		for(size_t i = 0; i < sphere.Vertices.size(); ++i)
		{
			vertices[i] = sphere.Vertices[i].Position;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];

		HR(device->CreateBuffer(&vbd, &vinitData, &mVB));


		mIndexCount = sphere.Indices.size();

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(USHORT) * mIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.StructureByteStride = 0;
		ibd.MiscFlags = 0;

		std::vector<USHORT> indices16;
		indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices16[0];

		HR(device->CreateBuffer(&ibd, &iinitData, &mIB));

		height_offset = 0.0f;
		height_scale = 1.0f;
	}
	~Sky()
	{
		ReleaseCOM(mVB);
		ReleaseCOM(mIB);
		ReleaseCOM(mCubeMapSRV);
	}

	ID3D11ShaderResourceView* CubeMapSRV()
	{
		return mCubeMapSRV;
	}

	void Draw(ID3D11DeviceContext* dc, Camera* camera)
	{
		// center Sky about eye in world space
		XMFLOAT3 eyePos = camera->GetPosition();
		XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y+height_offset, eyePos.z);

		XMMATRIX scale = XMMatrixScaling(1.0f, height_scale, 1.0f);
		XMMATRIX WVP = XMMatrixMultiply(scale*T, camera->ViewProj());

		ShaderManager* shaderManager = ShaderManager::getInstance();
		FXSkybox* fx = shaderManager->effects.fx_skybox;
		fx->SetWorldViewProj(WVP);
		fx->SetCubeMap(mCubeMapSRV);


		UINT stride = sizeof(XMFLOAT3);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);
		dc->IASetInputLayout(shaderManager->layout_pos);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3DX11_TECHNIQUE_DESC techDesc;
		fx->SkyTech->GetDesc( &techDesc );

		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			ID3DX11EffectPass* pass = fx->SkyTech->GetPassByIndex(p);

			pass->Apply(0, dc);

			dc->DrawIndexed(mIndexCount, 0, 0);
		}
	}

	void buildMenu(TwBar* menu)
	{
		TwAddVarRW(menu, "Sky height offset", TW_TYPE_FLOAT, &height_offset, "group=Sky step=1.00");
		TwAddVarRW(menu, "Sky height scale", TW_TYPE_FLOAT, &height_scale, "group=Sky step=0.01 min=0.01");
		TwDefine("Settings/Sky opened=false");
	};

private:
	Sky(const Sky& rhs);
	Sky& operator=(const Sky& rhs);
};

#endif // SKY_H