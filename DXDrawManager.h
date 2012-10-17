#ifndef DXDRAWMANAGER_H
#define DXDRAWMANAGER_H

#include <d3dx11.h>

#include "GeometryGenerator.h"
#include "Terrain.h"
#include "LightHelper.h"
#include "Game.h"
#include "ShaderManager.h"
#include <vector>

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

class DXDrawManager
{
private:
	ID3D11Device* dxDevice;
	ID3D11DeviceContext* dxDeviceContext;
	ShaderManager *shaderManager;

	FXStandard* fx;
	bool wireframe_enable;

	Terrain mTerrain;

	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;

	vector<Material> materials;

	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWavesMapSRV;
	XMFLOAT4X4 mGrassTexTransform;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;
	vector<int> vertexOffsets;
	vector<UINT> indexOffsets;
	vector<UINT> indexCounts;

	BoundingSphere mSceneBounds;

	ID3D11DepthStencilView* mDynamicCubeMapDSV;
	ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
	ID3D11ShaderResourceView* mDynamicCubeMapSRV;
	D3D11_VIEWPORT mCubeMapViewport;
	static const int CubeMapSize = 256;

protected:
public:
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	DXDrawManager(ID3D11Device* dxDevice, ID3D11DeviceContext* dxDeviceContext)
	{
		this->dxDevice = dxDevice;
		this->dxDeviceContext = dxDeviceContext;
		shaderManager = ShaderManager::getInstance();

		// Init geometry
		mShapesVB = 0;
		mShapesIB = 0;
		mScreenQuadVB = 0;
		mScreenQuadIB = 0;

		mDynamicCubeMapDSV = 0;
		mDynamicCubeMapSRV = 0;
		for(int i=0; i<6; i++)
			mDynamicCubeMapRTV[i] = 0;


		// Init light
		// directional light.
		mDirLight.Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLight.Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
		// Point light--position is changed every frame to animate in UpdateScene function.
		mPointLight.Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		mPointLight.Diffuse  = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		mPointLight.Att      = XMFLOAT3(0.0f, 0.1f, 0.0f);
		mPointLight.Position = XMFLOAT3(0.0f, 3.0f, 3.0f);
		mPointLight.Range    = 25.0f;

		// Spot light--position and direction changed every frame to animate in UpdateScene function.
		mSpotLight.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mSpotLight.Diffuse  = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mSpotLight.Att      = XMFLOAT3(1.0f, 0.0f, 0.0f);
		mSpotLight.Spot     = 96.0f;
		mSpotLight.Range    = 10000.0f;
		mSpotLight.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		mSpotLight.Position = XMFLOAT3(0.0f, 10.0f, 10.0f);

		// White
		if(true)
		{
			float specColor = 0.2f;
			float specIntencisty = 96.0f;
			Material mLandMat;
			mLandMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			mLandMat.Diffuse  = mLandMat.Ambient;
			mLandMat.Specular = XMFLOAT4(specColor, specColor,specColor, specIntencisty);
			materials.push_back(mLandMat);
		}

		// Land
		if(true)
		{
			float specColor = 0.2f;
			float specIntencisty = 96.0f;
			Material mLandMat;
			mLandMat.Ambient  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			mLandMat.Diffuse  = mLandMat.Ambient;
			mLandMat.Specular = XMFLOAT4(specColor, specColor,specColor, specIntencisty);
			materials.push_back(mLandMat);
		}
		if(true)
		{
			Material mLandMat;
			mLandMat.Ambient  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			mLandMat.Diffuse  = mLandMat.Ambient;
			mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 96.0f);
			materials.push_back(mLandMat);
		}
		
		// Maze
		if(true)
		{
			Material mMazeMat;
			mMazeMat.Ambient  = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			mMazeMat.Diffuse  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
			mMazeMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
			materials.push_back(mMazeMat);
		}
		
		// Box
		if(true)
		{
			Material mBoxMat;
			mBoxMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
			mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
			mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
			materials.push_back(mBoxMat);
		}

		XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
		XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

		mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mSceneBounds.Radius = 100.0f;

		buildGeometry();
		buildScreenQuadGeometry();

		mTerrain.Init(dxDevice, dxDeviceContext);
	};
	~DXDrawManager()
	{
		ReleaseCOM(mGrassMapSRV);
		ReleaseCOM(mWavesMapSRV);

		ReleaseCOM(mShapesVB);
		ReleaseCOM(mShapesIB);
		ReleaseCOM(mScreenQuadVB);
		ReleaseCOM(mScreenQuadIB);

		ReleaseCOM(mDynamicCubeMapDSV);
		ReleaseCOM(mDynamicCubeMapSRV);
		for(int i = 0; i < 6; ++i)
			ReleaseCOM(mDynamicCubeMapRTV[i]);
	};

	float getHillHeight( float x, float z )
	{
		return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
	}
	XMFLOAT3 getHillNormal( float x, float z )
	{
		// n = (-df/dx, 1, -df/dz)
		XMFLOAT3 n(
			-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
			1.0f,
			-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

		XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
		XMStoreFloat3(&n, unitNormal);

		return n;
	}
	void buildGeometry()
	{
		HR(D3DX11CreateShaderResourceViewFromFile(dxDevice, 
			L"Textures/grass.dds", 0, 0, &mGrassMapSRV, 0 ));

		HR(D3DX11CreateShaderResourceViewFromFile(dxDevice, 
			L"Textures/water2.dds", 0, 0, &mWavesMapSRV, 0 ));

		GeometryGenerator::MeshData box;
		GeometryGenerator::MeshData grid;
		GeometryGenerator::MeshData sphere;

		GeometryGenerator geoGen;
		geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
		geoGen.CreateSphere(0.5f, 20, 20, sphere);
		geoGen.CreateGrid(160.0f, 160.0f, 100, 100, grid);

		vertexOffsets.push_back(0);
		vertexOffsets.push_back(box.Vertices.size());
		vertexOffsets.push_back(vertexOffsets[1]+grid.Vertices.size());

		indexCounts.push_back(box.Indices.size());
		indexCounts.push_back(grid.Indices.size());
		indexCounts.push_back(sphere.Indices.size());

		indexOffsets.push_back(0);
		indexOffsets.push_back(indexCounts[0]);
		indexOffsets.push_back(indexOffsets[1] + indexOffsets[1]);

		UINT totalVertexCount = 
			box.Vertices.size() + 
			grid.Vertices.size() + 
			sphere.Vertices.size();

		UINT totalIndexCount = 0;
		for each(int i in indexCounts)
			totalIndexCount += i;

		//
		// Extract the vertex elements we are interested and apply the height function to
		// each vertex.  
		//
		std::vector<Vertex::posNormTex> vertices(totalVertexCount);
		UINT k = 0;
		for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos    = box.Vertices[i].Position;
			vertices[k].Normal = box.Vertices[i].Normal;
			vertices[k].Tex = box.Vertices[i].TexC;
		}

		for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;
			p.y = getHillHeight(p.x, p.z);
			vertices[k].Pos    = p;
			vertices[k].Normal = getHillNormal(p.x, p.z);
			vertices[k].Tex = grid.Vertices[i].TexC;
		}

		for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos    = sphere.Vertices[i].Position;
			vertices[k].Normal = sphere.Vertices[i].Normal;
			vertices[k].Tex = sphere.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex::posNormTex) * totalVertexCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(dxDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));


		//
		// Pack the indices of all the meshes into one index buffer.
		//

		std::vector<UINT> indices;
		indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
		indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
		indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
		HR(dxDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
	};
	void buildScreenQuadGeometry()
	{
		GeometryGenerator::MeshData quad;

		GeometryGenerator geoGen;
		geoGen.CreateFullscreenQuad(quad);

		//
		// Extract the vertex elements we are interested in and pack the
		// vertices of all the meshes into one vertex buffer.
		//

		std::vector<Vertex::posNormTex> vertices(quad.Vertices.size());

		for(UINT i = 0; i < quad.Vertices.size(); ++i)
		{
			vertices[i].Pos    = quad.Vertices[i].Position;
			vertices[i].Normal = quad.Vertices[i].Normal;
			vertices[i].Tex    = quad.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex::posNormTex) * quad.Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(dxDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &quad.Indices[0];
		HR(dxDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
	}
	void buildShadowTransform()
	{
		// Only the first "main" light casts a shadow.
		XMVECTOR lightDir = XMLoadFloat3(&mDirLight.Direction);
		XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
		XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

		// Transform bounding sphere to light space.
		XMFLOAT3 sphereCenterLS;
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

		// Ortho frustum in light space encloses scene.
		float l = sphereCenterLS.x - mSceneBounds.Radius;
		float b = sphereCenterLS.y - mSceneBounds.Radius;
		float n = sphereCenterLS.z - mSceneBounds.Radius;
		float r = sphereCenterLS.x + mSceneBounds.Radius;
		float t = sphereCenterLS.y + mSceneBounds.Radius;
		float f = sphereCenterLS.z + mSceneBounds.Radius;
		XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		XMMATRIX S = V*P*T;

		XMStoreFloat4x4(&mLightView, V);
		XMStoreFloat4x4(&mLightProj, P);
		XMStoreFloat4x4(&mShadowTransform, S);
	}
	void prepareFrame()
	{
		// Set input layout, topology, context
		dxDeviceContext->IASetInputLayout(shaderManager->layout_posNormTex);
		dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		UINT stride = sizeof(Vertex::posNormTex);
		UINT offset = 0;
		dxDeviceContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
		dxDeviceContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per frame constants.
		fx = shaderManager->effects.fx_standard;
		fx->SetDirLights(&mDirLight);
		fx->SetPointLights(&mPointLight);
		fx->SetSpotLights(&mSpotLight);
		fx->SetDiffuseMap(mWavesMapSRV);
	}
	void prepareFrame_shadowMap()
	{
		// Set input layout, topology, context
		dxDeviceContext->IASetInputLayout(shaderManager->layout_posNormTex);
		dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(Vertex::posNormTex);
		UINT offset = 0;
		dxDeviceContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
		dxDeviceContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);
	}
	void drawObject_shadowMap(int id_object, int id_material, CXMMATRIX world, CXMMATRIX viewProj, UINT passNr)
	{
		XMMATRIX worldViewProj = world*viewProj;

		FXBuildShadowMap* fx = shaderManager->effects.fx_buildShadowMap;
		fx->SetWorld(world);
		fx->SetWorldInvTranspose(Util::InverseTranspose(world));
		fx->SetWorldViewProj(worldViewProj);
		fx->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));

		dxDeviceContext->RSSetState(shaderManager->states.NoCullRS);
		fx->BuildShadowMapTech->GetPassByIndex(passNr)->Apply(0, dxDeviceContext);
		dxDeviceContext->DrawIndexed(indexCounts[id_object], indexOffsets[id_object], vertexOffsets[id_object]);
	}
	void drawObject(int id_object, int id_material, CXMMATRIX world, CXMMATRIX viewProj, UINT passNr)
	{
		XMMATRIX worldViewProj = world*viewProj;

		fx->SetWorld(world);
		fx->SetWorldViewProj(worldViewProj);
		fx->SetMaterial(materials[id_material]);
		fx->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		fx->SetShadowTransform(world*XMLoadFloat4x4(&mShadowTransform));

		fx->tech_light1->GetPassByIndex(passNr)->Apply(0, dxDeviceContext);
		dxDeviceContext->DrawIndexed(indexCounts[id_object], indexOffsets[id_object], vertexOffsets[id_object]);
	}

	void buildMenu(TwBar* menu)
	{
		// Lights
		TwAddVarRW(menu, "DirAmbient", TW_TYPE_COLOR4F, &mDirLight.Ambient, "group='Dir light'");
		TwAddVarRW(menu, "DirDiffuse", TW_TYPE_COLOR4F, &mDirLight.Diffuse, "group='Dir light'");
		TwAddVarRW(menu, "DirSpecular", TW_TYPE_COLOR4F, &mDirLight.Specular, "group='Dir light'");
		TwAddVarRW(menu, "DirDirection", TW_TYPE_DIR3F, &mDirLight.Direction, "group='Dir light' axisx=x axisy=z axisz=y");
		TwDefine("Settings/'Dir light' group=Lights");
		TwAddVarRW(menu, "PointAmbient", TW_TYPE_COLOR4F, &mPointLight.Ambient, "group='Point light'");
		TwAddVarRW(menu, "PointDiffuse", TW_TYPE_COLOR4F, &mPointLight.Diffuse, "group='Point light'");
		TwAddVarRW(menu, "PointSpecular", TW_TYPE_COLOR4F, &mPointLight.Specular, "group='Point light'");
		TwAddVarRW(menu, "PointAttenuation", TW_TYPE_COLOR3F, &mPointLight.Att, "group='Point light'");
		TwAddVarRW(menu, "PointRange", TW_TYPE_FLOAT, &mPointLight.Range, "group='Point light'");
		TwAddVarRW(menu, "PointPosition", TW_TYPE_DIR3F, &mPointLight.Position, "group='Point light'");
		TwDefine("Settings/'Point light' group=Lights");
		TwAddVarRW(menu, "SpotAmbient", TW_TYPE_COLOR4F, &mSpotLight.Ambient, "group='Spotlight'");
		TwAddVarRW(menu, "SpotDiffuse", TW_TYPE_COLOR4F, &mSpotLight.Diffuse, "group='Spotlight'");
		TwAddVarRW(menu, "SpotSpecular", TW_TYPE_COLOR4F, &mSpotLight.Specular, "group='Spotlight'");
		TwAddVarRW(menu, "SpotAttenuation", TW_TYPE_COLOR3F, &mSpotLight.Att, "group='Spotlight'");
		TwAddVarRW(menu, "SpotSpot", TW_TYPE_FLOAT, &mSpotLight.Spot, "group='Spotlight'");
		TwAddVarRW(menu, "SpotRange", TW_TYPE_FLOAT, &mSpotLight.Range, "group='Spotlight'");
		TwAddVarRW(menu, "SpotPosition", TW_TYPE_DIR3F, &mSpotLight.Position, "group='Spotlight'");
		TwAddVarRW(menu, "SpotDirection", TW_TYPE_DIR3F, &mSpotLight.Direction, "group='Spotlight'");
		TwDefine("Settings/'Spotlight' group=Lights");
		TwDefine("Settings/Lights opened=false");
	};
};

#endif //RENDERERDX_H