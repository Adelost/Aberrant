#ifndef DXRENDERER_H
#define DXRENDERER_H

#include <QWidget.h>
#include <d3dx11.h>

#include "DXDrawManager.h"
#include "Camera.h"
#include "Game.h"
#include "ShaderManager.h"
#include "ShadowMap.h"
#include "Sky.h"

class DXRenderer
{
private:
	int clientWidth;
	int clientHeight;
	HWND winId;

	ID3D11Device* dxDevice;
	ID3D11DeviceContext* dxDeviceContext;
	IDXGISwapChain* dxSwapChain;

	ID3D11RenderTargetView* view_renderTarget;
	ID3D11DepthStencilView* view_depthStencil;
	ID3D11Texture2D* tex_depthStencil;
	D3D11_VIEWPORT viewport_screen;

	ShaderManager *shaderManager;

	UINT msaa_quality;
	bool msaa_enable;
	bool wireframe_enable;

	Game pacman;
	TwBar *menu;
	bool lockCamera;

	ShadowMap* mSmap;
	static const int SMapSize = 2048;

	DXDrawManager *drawManager;
	Sky* mSky;
	Terrain mTerrain;

	float tess_heightScale;
	float tess_maxTessDistance;
	float tess_minTessDistance;
	float tess_minTessFactor;
	float tess_maxTessFactor;

public:
	Camera mCam;

protected:
private:
	void initDX();
	void resizeDX();
	void buildMenu();
public:
	DXRenderer();
	~DXRenderer();
	void init(HWND winId);
	void onResize(int width, int height);
	void recompileShaders()
	{
		shaderManager->effects.recompile(dxDevice);
	}

	float getAspectRatio();

	void initGameEntities();

	void update(float dt);
	void DrawSceneToShadowMap();

	void renderFrame();
	void drawGame(UINT pass);
	void DrawScreenQuad(ID3D11ShaderResourceView* resource);
};

#endif //RENDERERDX_H