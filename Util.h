#ifndef UTIL_H
#define UTIL_H

#include <d3dx11.h>
#include "d3dx11Effect.h"
#include "MathUtil.h"
#include "LightHelper.h"
// -- WARNING! Must include Windows.h before XNA or you will grow old and die!
#include <assert.h>
#include <sstream>
#include <fstream> // used to open files
#include <dxerr.h> // used to debug
#include <QMessageBox> // used to display info dialogs
#include <AntTweakBar.h> // used to debug

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

	XMGLOBALCONST XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
	XMGLOBALCONST XMVECTORF32 DeepBlue      = {0.14f, 0.42f, 0.56f, 1.0f};
}

class Util
{
public:
	template<typename T>
	static T clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}
	static XMMATRIX InverseTranspose(CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		XMMATRIX A = M;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixTranspose(XMMatrixInverse(&det, A));
	}

	// Order: left, right, bottom, top, near, far.
	static void extractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX mat)
	{
		//
		// Left
		//
		planes[0].x = mat(0,3) + mat(0,0);
		planes[0].y = mat(1,3) + mat(1,0);
		planes[0].z = mat(2,3) + mat(2,0);
		planes[0].w = mat(3,3) + mat(3,0);

		//
		// Right
		//
		planes[1].x = mat(0,3) - mat(0,0);
		planes[1].y = mat(1,3) - mat(1,0);
		planes[1].z = mat(2,3) - mat(2,0);
		planes[1].w = mat(3,3) - mat(3,0);

		//
		// Bottom
		//
		planes[2].x = mat(0,3) + mat(0,1);
		planes[2].y = mat(1,3) + mat(1,1);
		planes[2].z = mat(2,3) + mat(2,1);
		planes[2].w = mat(3,3) + mat(3,1);

		//
		// Top
		//
		planes[3].x = mat(0,3) - mat(0,1);
		planes[3].y = mat(1,3) - mat(1,1);
		planes[3].z = mat(2,3) - mat(2,1);
		planes[3].w = mat(3,3) - mat(3,1);

		//
		// Near
		//
		planes[4].x = mat(0,2);
		planes[4].y = mat(1,2);
		planes[4].z = mat(2,2);
		planes[4].w = mat(3,2);

		//
		// Far
		//
		planes[5].x = mat(0,3) - mat(0,2);
		planes[5].y = mat(1,3) - mat(1,2);
		planes[5].z = mat(2,3) - mat(2,2);
		planes[5].w = mat(3,3) - mat(3,2);

		// Normalize the plane equations.
		for(int i = 0; i < 6; ++i)
		{
			XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
			XMStoreFloat4(&planes[i], v);
		}
	}
};


//
// Macros
//

// DirectX error checking mecro
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
{                                                          \
	HRESULT hr = (x);                                      \
	if(FAILED(hr))                                         \
{                                                      \
	DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
}                                                      \
}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

// Convenience macros
#define ReleaseCOM(x){if(x){x->Release(); x = 0;}}
#define SafeDelete(x) { delete x; x = 0; }

class DXUtil
{
public:
	static ID3D11ShaderResourceView* create_view_texArray(
		ID3D11Device* device, 
		ID3D11DeviceContext* context,
		std::vector<std::wstring>& fileNames,
		DXGI_FORMAT format = DXGI_FORMAT_FROM_FILE,
		UINT filter = D3DX11_FILTER_NONE, 
		UINT mipFilter = D3DX11_FILTER_LINEAR)
	{
		//
		// Load the texture elements individually from file.  These textures
		// won't be used by the GPU (0 bind flags), they are just used to 
		// load the image data from file.  We use the STAGING usage so the
		// CPU can read the resource.
		//

		UINT size = fileNames.size();

		std::vector<ID3D11Texture2D*> srcTex(size);
		for(UINT i = 0; i < size; ++i)
		{
			D3DX11_IMAGE_LOAD_INFO loadInfo;

			loadInfo.Width  = D3DX11_FROM_FILE;
			loadInfo.Height = D3DX11_FROM_FILE;
			loadInfo.Depth  = D3DX11_FROM_FILE;
			loadInfo.FirstMipLevel = 0;
			loadInfo.MipLevels = D3DX11_FROM_FILE;
			loadInfo.Usage = D3D11_USAGE_STAGING;
			loadInfo.BindFlags = 0;
			loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
			loadInfo.MiscFlags = 0;
			loadInfo.Format = format;
			loadInfo.Filter = filter;
			loadInfo.MipFilter = mipFilter;
			loadInfo.pSrcInfo  = 0;

			HR(D3DX11CreateTextureFromFile(device, fileNames[i].c_str(), &loadInfo, NULL, (ID3D11Resource**)&srcTex[i], NULL));
		}


		//
		// Create texture array
		//

		// Get desc from first texture (each element has same format/dimension)
		D3D11_TEXTURE2D_DESC desc_texElement;
		srcTex[0]->GetDesc(&desc_texElement);

		D3D11_TEXTURE2D_DESC desc_texArray;
		desc_texArray.Width              = desc_texElement.Width;
		desc_texArray.Height             = desc_texElement.Height;
		desc_texArray.MipLevels          = desc_texElement.MipLevels;
		desc_texArray.ArraySize          = size;
		desc_texArray.Format             = desc_texElement.Format;
		desc_texArray.SampleDesc.Count   = 1;
		desc_texArray.SampleDesc.Quality = 0;
		desc_texArray.Usage              = D3D11_USAGE_DEFAULT;
		desc_texArray.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		desc_texArray.CPUAccessFlags     = 0;
		desc_texArray.MiscFlags          = 0;

		// Create array
		ID3D11Texture2D* texArray = 0;
		HR(device->CreateTexture2D( &desc_texArray, 0, &texArray));

		// Copy individual texture elements into texture array
		for(UINT texElement=0; texElement<size; texElement++)
		{
			for(UINT mipLevel=0; mipLevel<desc_texElement.MipLevels; mipLevel++)
			{
				D3D11_MAPPED_SUBRESOURCE mappedTex2D;
				HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

				context->UpdateSubresource(texArray, 
					D3D11CalcSubresource(mipLevel, texElement, desc_texElement.MipLevels),
					0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

				context->Unmap(srcTex[texElement], mipLevel);
			}
		}	


		//
		// Create´resource view to texture array.
		//

		D3D11_SHADER_RESOURCE_VIEW_DESC desc_view;
		desc_view.Format = desc_texArray.Format;
		desc_view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc_view.Texture2DArray.MostDetailedMip = 0;
		desc_view.Texture2DArray.MipLevels = desc_texArray.MipLevels;
		desc_view.Texture2DArray.FirstArraySlice = 0;
		desc_view.Texture2DArray.ArraySize = size;

		ID3D11ShaderResourceView* view_texArray = 0;
		HR(device->CreateShaderResourceView(texArray, &desc_view, &view_texArray));

		
		//
		// Cleanup -- we only need the resource view.
		//

		ReleaseCOM(texArray);
		for(UINT i = 0; i < size; ++i)
			ReleaseCOM(srcTex[i]);

		return view_texArray;
	}

	//static ID3D11ShaderResourceView* create_viewRandomTexture(ID3D11Device* device);
};


#endif // End