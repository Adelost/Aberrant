#ifndef TERRAIN_H
#define TERRAIN_H

#include "Util.h"
#include "LightHelper.h"
#include "ShaderManager.h"
#include "Camera.h"

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring path_heightMap;
		std::wstring path_blendMap;
		std::wstring path_layer0;
		std::wstring path_layer1;
		std::wstring path_layer2;
		std::wstring path_layer3;
		std::wstring path_layer4;
		float heightScale;
		float cellScale;
		UINT size_heightmap_x;
		UINT size_heightmap_y;
		UINT cellsPerPatch_dim;
	};

private:
	ID3D11Buffer* vbuff_patches;
	ID3D11Buffer* ibuff_patches;

	ID3D11ShaderResourceView* view_layersArray; 
	ID3D11ShaderResourceView* view_blendMap;
	ID3D11ShaderResourceView* view_heightMap;

	InitInfo info;

	XMFLOAT4X4 mWorld;

	Material mMat;

	// Height grid
	UINT num_vertex_x;
	UINT num_vertex_y;
	UINT num_cells_x;
	UINT num_cells_y;
	DynamicArray2D heightmap;
	float cellScale;

	// Patch grid
	UINT num_patchVertex_total;
	UINT num_patchCells_total;
	float cellsPerPatch_dim;
	float tess_min;
	UINT num_cellsPerPatch;
	UINT num_patchCells_x;
	UINT num_patchCells_y;
	UINT num_patchVertex_x;
	UINT num_patchVertex_y;
	std::vector<XMFLOAT2> heightmap_patchHeights;

	ID3D11Device* device;
	ID3D11DeviceContext* context;

public:
	Terrain()
	{
		vbuff_patches = 0;
		ibuff_patches = 0; 
		view_layersArray = 0; 
		view_blendMap = 0;
		view_heightMap = 0;
		tess_min = 2.0f;

		// Init attributes
		XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
		mMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
		mMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	};
	~Terrain()
	{
		ReleaseCOM(vbuff_patches);
		ReleaseCOM(ibuff_patches);
		ReleaseCOM(view_layersArray);
		ReleaseCOM(view_blendMap);
		ReleaseCOM(view_heightMap);
	};

	float getSize_x()
	{
		return num_cells_x*cellScale;
	}
	float getSize_y()
	{
		return num_cells_y*cellScale;
	}

	void init(ID3D11Device* device, ID3D11DeviceContext* context, Terrain::InitInfo info)
	{
		this->device = device;
		this->context = context;
		this->info = info;

		// Divide heightmap into patches of size "num_cellsPerPatch" cells
		cellScale = info.cellScale;
		num_vertex_x = info.size_heightmap_x;
		num_vertex_y = info.size_heightmap_y;
		// Note: adding +1 to cells gives us number of vertices in one dimension and vice versa
		num_cells_x = num_vertex_x - 1;
		num_cells_y = num_vertex_y - 1;
		
		// calc maximum patch size
		int max_numCellsPerPatch = MathUtil::gcd(num_cells_x, num_cells_y);
		// calc specified patch size -- 2^cellsPerPatch_dim
		cellsPerPatch_dim = info.cellsPerPatch_dim;
		num_cellsPerPatch = 1 << (int)cellsPerPatch_dim;
		// trim if necessary
		if(num_cellsPerPatch > max_numCellsPerPatch)
			num_cellsPerPatch = max_numCellsPerPatch;
		cellsPerPatch_dim++;

		num_patchCells_x = num_cells_x/num_cellsPerPatch;
		num_patchCells_y = num_cells_y/num_cellsPerPatch;
		num_patchVertex_x = num_patchCells_x + 1;
		num_patchVertex_y = num_patchCells_y + 1;

		num_patchVertex_total  = num_patchVertex_x*num_patchVertex_y;
		num_patchCells_total = num_patchCells_x*num_patchCells_y;

		// Create heightmap
		loadHeightmap(info.path_heightMap, info.heightScale);
		heightmap.smooth();
		calc_patchHeights();

		buildQuadPatchVB(device);
		buildQuadPatchIB(device);
		buildHeightmapSRV(device);

		// Create texture array of blendmaps
		std::vector<std::wstring> layerFilenames;
		layerFilenames.push_back(info.path_layer0);
		layerFilenames.push_back(info.path_layer1);
		layerFilenames.push_back(info.path_layer2);
		layerFilenames.push_back(info.path_layer3);
		layerFilenames.push_back(info.path_layer4);
		view_layersArray = DXUtil::create_view_texArray(device, 
			context, layerFilenames);

		HR(D3DX11CreateShaderResourceViewFromFile(device,
			info.path_blendMap.c_str(), 0, 0, &view_blendMap, 0));
	}

	void draw(ID3D11DeviceContext* dc, Camera *cam)
	{
		ShaderManager* sm = ShaderManager::getInstance();
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		dc->IASetInputLayout(sm->layout_posTexBoundY);
		

		UINT stride = sizeof(Vertex::posTexBondsY);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, &vbuff_patches, &stride, &offset);
		dc->IASetIndexBuffer(ibuff_patches, DXGI_FORMAT_R16_UINT, 0);

		XMMATRIX viewProj = cam->ViewProj();
		XMMATRIX world  = XMLoadFloat4x4(&mWorld);
		XMMATRIX worldViewProj = world*viewProj;

		// Convert frustum into  6 planes
		XMFLOAT4 worldPlanes[6];
		Util::extractFrustumPlanes(worldPlanes, cam->ViewProjDebug());

		// Set per frame constants.
		FXStandard* fx = sm->effects.fx_standard;
		fx->SetViewProj(viewProj);
		fx->SetEyePosW(cam->GetPosition());
	
		fx->SetMinTessDistance(20.0f);
		fx->SetMaxTessDistance(500.0f);
		fx->SetMinTessFactor(tess_min);
		fx->SetMaxTessFactor(cellsPerPatch_dim);

		fx->SetTexelCellSpaceU(1.0f / num_vertex_x);
		fx->SetTexelCellSpaceV(1.0f / num_vertex_y);
		fx->SetWorldCellSpace(cellScale);
		fx->SetWorldFrustumPlanes(worldPlanes);

		fx->SetLayerMapArray(view_layersArray);
		fx->SetBlendMap(view_blendMap);
		fx->SetHeightMap(view_heightMap);

		fx->SetMaterial(mMat);

		ID3DX11EffectTechnique* tech = sm->effects.fx_standard->tech_terrain;
		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc( &techDesc );

		for(UINT i = 0; i < techDesc.Passes; ++i)
		{
			ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
			pass->Apply(0, dc);

			dc->DrawIndexed(num_patchCells_total*4, 0, 0);
		}	

		// FX sets tessellation stages, but it does not disable them.  So do that here
		// to turn off tessellation.
		dc->HSSetShader(0, 0, 0);
		dc->DSSetShader(0, 0, 0);
	}
	float getTerrainHeight(float x, float z)
	{
		// Transform from terrain local space to "cell" space.
		float c = (x + 0.5f*getSize_x()) /  cellScale;
		float d = (z - 0.5f*getSize_y()) / -cellScale;

		// Get the row and column we are in.
		int row = (int)floorf(d);
		int col = (int)floorf(c);

		// Grab the heights of the cell we are in.
		float A = heightmap.safe_get(col, row);
		float B = heightmap.safe_get(col+1, row);
		float C = heightmap.safe_get(col, row+1);
		float D = heightmap.safe_get(col+1, row+1);

		// Where we are relative to the cell.
		float s = c - (float)col;
		float t = d - (float)row;

		// If upper triangle ABC.
		if( s + t <= 1.0f)
		{
			float uy = B - A;
			float vy = C - A;
			return A + s*uy + t*vy;
		}
		else // lower triangle DCB.
		{
			float uy = C - D;
			float vy = B - D;
			return D + (1.0f-s)*uy + (1.0f-t)*vy;
		}
	}

	void recreate()
	{
		init(device, context, info);
	}

	void buildMenu(TwBar* menu);

private:
	void loadHeightmap(std::wstring path, float heightScale)
	{
		// Read bytes from RAW file
		std::vector<unsigned char> byte_array(num_vertex_x * num_vertex_y);
		std::ifstream inFile;
		inFile.open(path.c_str(), std::ios_base::binary);
		if(inFile)
		{
			inFile.read((char*)&byte_array[0], (std::streamsize)byte_array.size());
			inFile.close();
		}

		// Copy the array data into a float array and scale it.
		heightmap.resize(num_vertex_x, num_vertex_y);
		for(int i=0; i<heightmap.size_total; i++)
		{
			// convert from byte to float
			float height = (byte_array[i]/255.0f)*heightScale;
			heightmap.set(i, height);
		}
	};
	
	void calc_patchHeights()
	{
		heightmap_patchHeights.resize(num_patchCells_total);

		// For each patch
		for(UINT y=0; y<num_patchCells_y; y++)
			for(UINT x=0; x<num_patchCells_x; x++)
				calc_patchHeights(x, y);
	}
	void calc_patchHeights(UINT ix, UINT iy)
	{
		//
		// Compute min max value in each patch
		//

		// Start/End index for each patch
		// Note: x0 = start, x1 = end
		UINT x0 = ix*num_cellsPerPatch;
		UINT x1 = (ix+1)*num_cellsPerPatch;
		UINT y0 = iy*num_cellsPerPatch;
		UINT y1 = (iy+1)*num_cellsPerPatch;

		float min_heigh = +FLT_MAX;
		float max_heigh = -FLT_MAX;
		for(UINT y=y0; y<=y1; y++)
		{
			for(UINT x=x0; x<=x1; x++)
			{
				float height = heightmap.get(x,y);
				min_heigh = MathUtil::Min(min_heigh, height);
				max_heigh = MathUtil::Max(max_heigh, height);
			}
		}

		int index = ix+iy*num_patchCells_x;
		heightmap_patchHeights[index] = XMFLOAT2(min_heigh, max_heigh);
	}
	void buildQuadPatchVB(ID3D11Device* device)
	{
		std::vector<Vertex::posTexBondsY> patchVertices(num_patchVertex_total);

		// Calc position and texture-cords
		float halfSize_x = 0.5f*getSize_x();
		float halfSize_y = 0.5f*getSize_y();
		float size_patchCell_x = getSize_x() / num_patchCells_x;
		float size_patchCell_y = getSize_y() / num_patchCells_y;
		float du = 1.0f / num_patchCells_x;
		float dv = 1.0f / num_patchCells_y;
		for(int y=0; y<num_patchVertex_y; y++)
		{
			// Y-position -- we invert axis because image is inverted
			float pos_y = - y*size_patchCell_y + halfSize_y;
			for(int x=0; x<num_patchVertex_x; x++)
			{
				// Position vertex on grid relative to center
				float pos_x =  x*size_patchCell_x - halfSize_x;
				patchVertices[x+y*num_patchVertex_x].Pos = XMFLOAT3(pos_x, 0.0f, pos_y);

				// Stretch texture over grid.
				patchVertices[x+y*num_patchVertex_x].Tex.x = x*du;
				patchVertices[x+y*num_patchVertex_x].Tex.y = y*dv;
			}
		}

		// Store axis-aligned bounding box y-bounds in upper-left patch corner.
		// Note: lower left corner is left empty as it is not needed
		for(UINT y=0; y<num_patchCells_y; y++)
		{
			for(UINT x=0; x<num_patchCells_x; x++)
			{
				UINT index = x+y*num_patchCells_x;
				patchVertices[x+y*num_patchVertex_x].BoundsY = heightmap_patchHeights[index];
			}
		}


		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex::posTexBondsY) * patchVertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &patchVertices[0];

		std::vector<int> debug(patchVertices.size());
		for(UINT i = 0; i < patchVertices.size(); ++i)
		{
			debug[i] =  patchVertices [i].Pos.z;
		}

		HR(device->CreateBuffer(&vbd, &vinitData, &vbuff_patches));
	}
	void buildQuadPatchIB(ID3D11Device* device)
	{
		std::vector<USHORT> indices(num_patchCells_total*4); // 4 indices per quad face
		
		// Iterate over each quad and compute indices.
		int k = 0;
		for(UINT i = 0; i < num_patchCells_y; ++i)
		{
			for(UINT j = 0; j < num_patchCells_x; ++j)
			{
				// Top row of 2x2 quad patch
				indices[k]   = i*num_patchVertex_x+j;
				indices[k+1] = i*num_patchVertex_x+j+1;

				// Bottom row of 2x2 quad patch
				indices[k+2] = (i+1)*num_patchVertex_x+j;
				indices[k+3] = (i+1)*num_patchVertex_x+j+1;

				k += 4; // next quad
			}
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(USHORT) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];

		HR(device->CreateBuffer(&ibd, &iinitData, &ibuff_patches));
	}
	void buildHeightmapSRV(ID3D11Device* device)
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = num_vertex_x;
		texDesc.Height = num_vertex_y;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format    = DXGI_FORMAT_R16_FLOAT;
		texDesc.SampleDesc.Count   = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		// HALF is defined in xnamath.h, for storing 16-bit float.
		std::vector<HALF> hmap(heightmap.size_total);
		for(UINT i=0; i<hmap.size(); i++)
			hmap[i]=XMConvertFloatToHalf(heightmap.get(i));

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &hmap[0];
		data.SysMemPitch = num_vertex_x*sizeof(HALF);
		data.SysMemSlicePitch = 0;

		ID3D11Texture2D* hmapTex = 0;
		HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &view_heightMap));

		// SRV saves reference.
		ReleaseCOM(hmapTex);
	}
};

#endif // TERRAIN_H