#ifndef TERRAIN_H
#define TERRAIN_H

#include "Util.h"
#include "LightHelper.h"
#include "ShaderManager.h"

#include <fstream>
#include <sstream>

class Camera;
struct DirectionalLight;

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};

private:
	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices.  Use 64 so that if we tessellate all the way 
	// to 64, we use all the data from the heightmap.  
	static const int CellsPerPatch = 64;

	ID3D11Buffer* mQuadPatchVB;
	ID3D11Buffer* mQuadPatchIB;

	ID3D11ShaderResourceView* mLayerMapArraySRV;
	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;

	Material mMat;

	std::vector<XMFLOAT2> mPatchBoundsY;
	std::vector<float> mHeightmap;

public:
	Terrain()
	{
		mQuadPatchVB = 0;
		mQuadPatchIB = 0; 
		mLayerMapArraySRV = 0; 
		mBlendMapSRV = 0;
		mHeightMapSRV = 0;
		mNumPatchVertices = 0;
		mNumPatchQuadFaces = 0;
		mNumPatchVertRows = 0;
		mNumPatchVertCols = 0;

		XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
		mMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
		mMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	};
	~Terrain()
	{
		ReleaseCOM(mQuadPatchVB);
		ReleaseCOM(mQuadPatchIB);
		ReleaseCOM(mLayerMapArraySRV);
		ReleaseCOM(mBlendMapSRV);
		ReleaseCOM(mHeightMapSRV);
	};

	float GetWidth()
	{
		// Total terrain width.
		return (mInfo.HeightmapWidth-1)*mInfo.CellSpacing;
	}
	float GetDepth()
	{
		// Total terrain depth.
		return (mInfo.HeightmapHeight-1)*mInfo.CellSpacing;
	}
	float GetHeight(float x, float z);

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc)
	{
		Terrain::InitInfo tii;
		tii.HeightMapFilename = L"Textures/Terrain/terrain.raw";
		tii.LayerMapFilename0 = L"Textures/Terrain/grass.dds";
		tii.LayerMapFilename1 = L"Textures/Terrain/darkdirt.dds";
		tii.LayerMapFilename2 = L"Textures/Terrain/stone.dds";
		tii.LayerMapFilename3 = L"Textures/Terrain/lightdirt.dds";
		tii.LayerMapFilename4 = L"Textures/Terrain/snow.dds";
		tii.BlendMapFilename = L"Textures/Terrain/blend.dds";
		tii.HeightScale = 50.0f;
		tii.HeightmapWidth = 2049;
		tii.HeightmapHeight = 2049;
		tii.CellSpacing = 0.5f;

		mInfo = tii;

		// Divide heightmap into patches such that each patch has CellsPerPatch.
		mNumPatchVertRows = ((mInfo.HeightmapHeight-1) / CellsPerPatch) + 1;
		mNumPatchVertCols = ((mInfo.HeightmapWidth-1) / CellsPerPatch) + 1;

		mNumPatchVertices  = mNumPatchVertRows*mNumPatchVertCols;
		mNumPatchQuadFaces = (mNumPatchVertRows-1)*(mNumPatchVertCols-1);

		LoadHeightmap();
		Smooth();
		CalcAllPatchBoundsY();

		BuildQuadPatchVB(device);
		BuildQuadPatchIB(device);
		BuildHeightmapSRV(device);

		std::vector<std::wstring> layerFilenames;
		layerFilenames.push_back(mInfo.LayerMapFilename0);
		layerFilenames.push_back(mInfo.LayerMapFilename1);
		layerFilenames.push_back(mInfo.LayerMapFilename2);
		layerFilenames.push_back(mInfo.LayerMapFilename3);
		layerFilenames.push_back(mInfo.LayerMapFilename4);
		mLayerMapArraySRV = DXUtil::CreateTexture2DArraySRV(device, dc, layerFilenames);

		HR(D3DX11CreateShaderResourceViewFromFile(device, 
			mInfo.BlendMapFilename.c_str(), 0, 0, &mBlendMapSRV, 0));
	}

	void Draw(ID3D11DeviceContext* dc, const Camera& cam)
	{

	}

private:
	void LoadHeightmap()
	{
		// A height for each vertex
		std::vector<unsigned char> in( mInfo.HeightmapWidth * mInfo.HeightmapHeight );

		// Open the file.
		std::ifstream inFile;
		inFile.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);

		if(inFile)
		{
			// Read the RAW bytes.
			inFile.read((char*)&in[0], (std::streamsize)in.size());

			// Done with file.
			inFile.close();
		}

		// Copy the array data into a float array and scale it.
		mHeightmap.resize(mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0);
		for(UINT i = 0; i < mInfo.HeightmapHeight * mInfo.HeightmapWidth; ++i)
		{
			mHeightmap[i] = (in[i] / 255.0f)*mInfo.HeightScale;
		}
	};
	void Smooth()
	{
		std::vector<float> dest( mHeightmap.size() );

		for(UINT i = 0; i < mInfo.HeightmapHeight; ++i)
		{
			for(UINT j = 0; j < mInfo.HeightmapWidth; ++j)
			{
				dest[i*mInfo.HeightmapWidth+j] = Average(i,j);
			}
		}

		// Replace the old heightmap with the filtered one.
		mHeightmap = dest;
	}
	bool InBounds(int i, int j)
	{
		// True if ij are valid indices; false otherwise.
		return 
			i >= 0 && i < (int)mInfo.HeightmapHeight && 
			j >= 0 && j < (int)mInfo.HeightmapWidth;
	}
	float Average(int i, int j)
	{
		// Function computes the average height of the ij element.
		// It averages itself with its eight neighbor pixels.  Note
		// that if a pixel is missing neighbor, we just don't include it
		// in the average--that is, edge pixels don't have a neighbor pixel.
		//
		// ----------
		// | 1| 2| 3|
		// ----------
		// |4 |ij| 6|
		// ----------
		// | 7| 8| 9|
		// ----------

		float avg = 0.0f;
		float num = 0.0f;

		// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
		// and no iterations of the outer for loop occur.
		for(int m = i-1; m <= i+1; ++m)
		{
			for(int n = j-1; n <= j+1; ++n)
			{
				if( InBounds(m,n) )
				{
					avg += mHeightmap[m*mInfo.HeightmapWidth + n];
					num += 1.0f;
				}
			}
		}

		return avg / num;
	}
	void CalcAllPatchBoundsY()
	{
		mPatchBoundsY.resize(mNumPatchQuadFaces);

		// For each patch
		for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
		{
			for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
			{
				CalcPatchBoundsY(i, j);
			}
		}
	}
	void CalcPatchBoundsY(UINT i, UINT j)
	{
		// Scan the heightmap values this patch covers and compute the min/max height.

		UINT x0 = j*CellsPerPatch;
		UINT x1 = (j+1)*CellsPerPatch;

		UINT y0 = i*CellsPerPatch;
		UINT y1 = (i+1)*CellsPerPatch;

		float minY = +FLT_MAX;
		float maxY = -FLT_MAX;
		for(UINT y = y0; y <= y1; ++y)
		{
			for(UINT x = x0; x <= x1; ++x)
			{
				UINT k = y*mInfo.HeightmapWidth + x;
				minY = MathUtil::Min(minY, mHeightmap[k]);
				maxY = MathUtil::Max(maxY, mHeightmap[k]);
			}
		}

		UINT patchID = i*(mNumPatchVertCols-1)+j;
		mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
	}
	void BuildQuadPatchVB(ID3D11Device* device)
	{
		std::vector<Vertex::posTexBondsY> patchVertices(mNumPatchVertRows*mNumPatchVertCols);

		float halfWidth = 0.5f*GetWidth();
		float halfDepth = 0.5f*GetDepth();

		float patchWidth = GetWidth() / (mNumPatchVertCols-1);
		float patchDepth = GetDepth() / (mNumPatchVertRows-1);
		float du = 1.0f / (mNumPatchVertCols-1);
		float dv = 1.0f / (mNumPatchVertRows-1);

		for(UINT i = 0; i < mNumPatchVertRows; ++i)
		{
			float z = halfDepth - i*patchDepth;
			for(UINT j = 0; j < mNumPatchVertCols; ++j)
			{
				float x = -halfWidth + j*patchWidth;

				patchVertices[i*mNumPatchVertCols+j].Pos = XMFLOAT3(x, 0.0f, z);

				// Stretch texture over grid.
				patchVertices[i*mNumPatchVertCols+j].Tex.x = j*du;
				patchVertices[i*mNumPatchVertCols+j].Tex.y = i*dv;
			}
		}

		// Store axis-aligned bounding box y-bounds in upper-left patch corner.
		for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
		{
			for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
			{
				UINT patchID = i*(mNumPatchVertCols-1)+j;
				patchVertices[i*mNumPatchVertCols+j].BoundsY = mPatchBoundsY[patchID];
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
		HR(device->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
	}
	void BuildQuadPatchIB(ID3D11Device* device)
	{
		std::vector<USHORT> indices(mNumPatchQuadFaces*4); // 4 indices per quad face

		// Iterate over each quad and compute indices.
		int k = 0;
		for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
		{
			for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
			{
				// Top row of 2x2 quad patch
				indices[k]   = i*mNumPatchVertCols+j;
				indices[k+1] = i*mNumPatchVertCols+j+1;

				// Bottom row of 2x2 quad patch
				indices[k+2] = (i+1)*mNumPatchVertCols+j;
				indices[k+3] = (i+1)*mNumPatchVertCols+j+1;

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
		HR(device->CreateBuffer(&ibd, &iinitData, &mQuadPatchIB));
	}
	void BuildHeightmapSRV(ID3D11Device* device)
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = mInfo.HeightmapWidth;
		texDesc.Height = mInfo.HeightmapHeight;
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
		std::vector<HALF> hmap(mHeightmap.size());
		std::transform(mHeightmap.begin(), mHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &hmap[0];
		data.SysMemPitch = mInfo.HeightmapWidth*sizeof(HALF);
		data.SysMemSlicePitch = 0;

		ID3D11Texture2D* hmapTex = 0;
		HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &mHeightMapSRV));

		// SRV saves reference.
		ReleaseCOM(hmapTex);
	}
};

#endif // TERRAIN_H