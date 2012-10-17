#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
#include <xnamath.h> // math library
#include <sstream>
#include <fstream> // used to open files
#include <dxerr.h> // used to debug
#include <QMessageBox> // used to display info dialogs
#include <AntTweakBar.h> // used to debug

#include "MathUtil.h"

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
	static ID3D11ShaderResourceView* CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
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

		UINT size = filenames.size();

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

			HR(D3DX11CreateTextureFromFile(device, filenames[i].c_str(), 
				&loadInfo, 0, (ID3D11Resource**)&srcTex[i], 0));
		}

		//
		// Create the texture array.  Each element in the texture 
		// array has the same format/dimensions.
		//

		D3D11_TEXTURE2D_DESC texElementDesc;
		srcTex[0]->GetDesc(&texElementDesc);

		D3D11_TEXTURE2D_DESC texArrayDesc;
		texArrayDesc.Width              = texElementDesc.Width;
		texArrayDesc.Height             = texElementDesc.Height;
		texArrayDesc.MipLevels          = texElementDesc.MipLevels;
		texArrayDesc.ArraySize          = size;
		texArrayDesc.Format             = texElementDesc.Format;
		texArrayDesc.SampleDesc.Count   = 1;
		texArrayDesc.SampleDesc.Quality = 0;
		texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
		texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		texArrayDesc.CPUAccessFlags     = 0;
		texArrayDesc.MiscFlags          = 0;

		ID3D11Texture2D* texArray = 0;
		HR(device->CreateTexture2D( &texArrayDesc, 0, &texArray));

		//
		// Copy individual texture elements into texture array.
		//

		// for each texture element...
		for(UINT texElement = 0; texElement < size; ++texElement)
		{
			// for each mipmap level...
			for(UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
			{
				D3D11_MAPPED_SUBRESOURCE mappedTex2D;
				HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

				context->UpdateSubresource(texArray, 
					D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
					0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

				context->Unmap(srcTex[texElement], mipLevel);
			}
		}	

		//
		// Create a resource view to the texture array.
		//

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = texArrayDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		viewDesc.Texture2DArray.MostDetailedMip = 0;
		viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
		viewDesc.Texture2DArray.FirstArraySlice = 0;
		viewDesc.Texture2DArray.ArraySize = size;

		ID3D11ShaderResourceView* texArraySRV = 0;
		HR(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

		//
		// Cleanup--we only need the resource view.
		//

		ReleaseCOM(texArray);

		for(UINT i = 0; i < size; ++i)
			ReleaseCOM(srcTex[i]);

		return texArraySRV;
	}

	//static ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
};


#endif // End