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
#include "Sound.h"

static bool msaa_enable = false;

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
	
	bool wireframe_enable;

	Game pacman;
	TwBar *menu;
	bool lockCamera;
	bool lockPacmanCamera;

	ShadowMap* mSmap;
	static const int SMapSize = 2048;

	DXDrawManager *drawManager;
	Sky* mSky;
	Terrain mTerrain;

	// Tessellation
	float tess_heightScale;
	float tess_maxTessDistance;
	float tess_minTessDistance;
	float tess_minTessFactor;
	float tess_maxTessFactor;

	float mesh_maxTessFactor;
	float mesh_heightScale;

	std::vector<Vertex::InstancedData> instancedData;
	ID3D11Buffer* instancedBuffer;
	int num_visibleObjects;

	// Settings
	bool drawPacman;
	bool drawTerrain;
	bool drawPlane;
	bool drawSky;
	bool drawMesh;

	// Sound 
	// -- disclaimers, mem leak when creating sound buffers, 
	// havn't got time to figure out why
	Sound sound;
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
	void init(HWND winId, bool* doMSAA);
	void onResize(int width, int height);
	void recompileShaders()
	{
		shaderManager->effects.recompile(dxDevice);
	}

	float getAspectRatio();

	void initGameEntities();
	void initInstanceBuffer();

	void update(float dt);
	void DrawSceneToShadowMap();

	void renderFrame();
	void drawGame();
	void DrawScreenQuad(ID3D11ShaderResourceView* resource);

};

#endif //RENDERERDX_H