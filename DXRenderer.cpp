#include "DXRenderer.h"
#include "Effects.h"


DXRenderer::DXRenderer()
{
	// DX init
	dxDevice = 0;
	dxDeviceContext = 0,
	dxSwapChain = 0;
	tex_depthStencil = 0;
	view_renderTarget = 0;
	view_depthStencil = 0;
	mSmap = 0;
	instancedBuffer = 0;

	// DX settings
	msaa_quality = 0;
	msaa_enable = true;
	wireframe_enable = false;
	clientWidth = 800;
	clientHeight = 600;
	drawPacman  = false;
	drawTerrain  = true;
	drawPlane   = false;
	drawSky   = true;
	drawMesh   = false;

	tess_heightScale = 10.7f;
	tess_maxTessDistance = 5.0f;
	tess_minTessDistance = 200.0f;
	tess_minTessFactor = 1.0f;
	tess_maxTessFactor = 7.0f;
}

DXRenderer::~DXRenderer()
{
	// Release DX
	ReleaseCOM(view_renderTarget);
	ReleaseCOM(view_depthStencil);
	ReleaseCOM(dxSwapChain);
	ReleaseCOM(tex_depthStencil);
	if(dxDeviceContext)
		dxDeviceContext->ClearState();
	ReleaseCOM(dxDeviceContext);
	ReleaseCOM(dxDevice);
	ReleaseCOM(instancedBuffer);

	// Delete managers
	delete drawManager;
	delete mSky;
	SafeDelete(mSmap);
	shaderManager->~ShaderManager();

	// Terminate tweakbar
	TwTerminate();
}

void DXRenderer::init(HWND winId)
{
	// Init DX
	this->winId = winId;
	initDX();
	shaderManager = ShaderManager::getInstance();
	shaderManager->init(dxDevice);
	drawManager = new DXDrawManager(dxDevice, dxDeviceContext);
	mSky = new Sky(dxDevice, L"Textures/Skyboxes/plain.dds", 5000.0f);
	mSmap = new ShadowMap(dxDevice, SMapSize, SMapSize);

	Terrain::InitInfo info;
	info.path_heightMap = L"Textures/Terrain/_terrain.raw";
	info.path_blendMap = L"Textures/Terrain/_blend.dds";
	info.path_layer0 = L"Textures/Terrain/default_white.dds";
	info.path_layer1 = L"Textures/Terrain/default_white.dds";
	info.path_layer2 = L"Textures/Terrain/default_white.dds";
	info.path_layer3 = L"Textures/Terrain/road.dds";
	info.path_layer4 = L"Textures/Terrain/default_green.dds";
	info.heightScale = 50.0f;
	info.cellScale = 0.5f;
	info.size_heightmap_x = 2049;
	info.size_heightmap_y = 2049;
	info.cellsPerPatch_dim = 6;

	mTerrain.init(dxDevice, dxDeviceContext, info);
	//sound.init();

	mesh_maxTessFactor = 1.0f;
	mesh_heightScale = 0.0f;

	// Init tweakbar
	TwInit(TW_DIRECT3D11, dxDevice);
	TwWindowSize(clientWidth, clientHeight);

	// Init game
	initGameEntities();
	initInstanceBuffer();
	buildMenu();
}

void TW_CALL tw_recompileShaders(void *clientData)
{ 
	DXRenderer *in = static_cast<DXRenderer *>(clientData); // scene pointer is stored in clientData
	in->recompileShaders();                            
}

void DXRenderer::buildMenu()
{
	// Create menu in renderer
	menu = TwNewBar("Settings");
	TwDefine("Settings contained=true position='0 0'"); 

	// Display
	TwAddVarRW(menu, "Wire frame", TW_TYPE_BOOLCPP, &wireframe_enable, "group=Display");
	TwAddVarRW(menu, "MSAA", TW_TYPE_BOOLCPP, &msaa_enable, "group=Display");
	TwAddButton(menu, "Recompile shaders", tw_recompileShaders, this, "group=Display");
	TwDefine("Settings/Display opened=false");
	TwAddSeparator(menu, NULL, NULL);

	// Render
	TwAddVarRW(menu, "Render terrain", TW_TYPE_BOOLCPP, &drawTerrain, "group=Render");
	TwAddVarRW(menu, "Render plane", TW_TYPE_BOOLCPP, &drawPlane, "group=Render");
	TwAddVarRW(menu, "Render pacman", TW_TYPE_BOOLCPP, &drawPacman, "group=Render");
	TwAddVarRW(menu, "Render sky", TW_TYPE_BOOLCPP, &drawSky, "group=Render");
	TwAddVarRW(menu, "Render mesh", TW_TYPE_BOOLCPP, &drawMesh, "group=Render");
	TwDefine("Settings/Render opened=false");

	//// Lights
	//TwAddVarRW(menu, "DirAmbient", TW_TYPE_COLOR4F, &mDirLight.Ambient, "group='Dir light'");
	//TwAddVarRW(menu, "DirDiffuse", TW_TYPE_COLOR4F, &mDirLight.Diffuse, "group='Dir light'");
	//TwAddVarRW(menu, "DirSpecular", TW_TYPE_COLOR4F, &mDirLight.Specular, "group='Dir light'");
	//TwAddVarRW(menu, "DirDirection", TW_TYPE_DIR3F, &mDirLight.Direction, "group='Dir light'");
	//TwDefine("Settings/'Dir light' group=Lights");
	//TwAddVarRW(menu, "PointAmbient", TW_TYPE_COLOR4F, &mPointLight.Ambient, "group='Point light'");
	//TwAddVarRW(menu, "PointDiffuse", TW_TYPE_COLOR4F, &mPointLight.Diffuse, "group='Point light'");
	//TwAddVarRW(menu, "PointSpecular", TW_TYPE_COLOR4F, &mPointLight.Specular, "group='Point light'");
	//TwAddVarRW(menu, "PointAttenuation", TW_TYPE_COLOR3F, &mPointLight.Att, "group='Point light'");
	//TwAddVarRW(menu, "PointRange", TW_TYPE_FLOAT, &mPointLight.Range, "group='Point light'");
	//TwAddVarRW(menu, "PointPosition", TW_TYPE_DIR3F, &mPointLight.Position, "group='Point light'");
	//TwDefine("Settings/'Point light' group=Lights");
	//TwAddVarRW(menu, "SpotAmbient", TW_TYPE_COLOR4F, &mSpotLight.Ambient, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotDiffuse", TW_TYPE_COLOR4F, &mSpotLight.Diffuse, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotSpecular", TW_TYPE_COLOR4F, &mSpotLight.Specular, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotAttenuation", TW_TYPE_COLOR3F, &mSpotLight.Att, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotSpot", TW_TYPE_FLOAT, &mSpotLight.Spot, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotRange", TW_TYPE_FLOAT, &mSpotLight.Range, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotPosition", TW_TYPE_DIR3F, &mSpotLight.Position, "group='Spotlight'");
	//TwAddVarRW(menu, "SpotDirection", TW_TYPE_DIR3F, &mSpotLight.Direction, "group='Spotlight'");
	//TwDefine("Settings/'Spotlight' group=Lights");
	//TwDefine("Settings/Lights opened=false");

	////Material
	//TwAddVarRW(menu, "BoxAmbient", TW_TYPE_COLOR4F, &mBoxMat.Ambient, "group='Box Material'");
	//TwAddVarRW(menu, "BoxDiffuse", TW_TYPE_COLOR4F, &mBoxMat.Diffuse, "group='Box Material'");
	//TwAddVarRW(menu, "BoxSpecular", TW_TYPE_COLOR4F, &mBoxMat.Specular, "group='Box Material'");
	//TwAddVarRW(menu, "BoxSpecPower", TW_TYPE_FLOAT, &mBoxMat.Specular.w, "group='Box Material' min=16.0f");
	//TwAddVarRW(menu, "MazeAmbient", TW_TYPE_COLOR4F, &mMazeMat.Ambient, "group='Maze Material'");
	//TwAddVarRW(menu, "MazeDiffuse", TW_TYPE_COLOR4F, &mMazeMat.Diffuse, "group='Maze Material'");
	//TwAddVarRW(menu, "MazeSpecular", TW_TYPE_COLOR4F, &mMazeMat.Specular, "group='Maze Material'");
	//TwAddVarRW(menu, "MazeSpecPower", TW_TYPE_FLOAT, &mMazeMat.Specular.w, "group='Maze Material' min=16.0f");
	//TwAddVarRW(menu, "LandAmbient", TW_TYPE_COLOR4F, &mLandMat.Ambient, "group='Land Material'");
	//TwAddVarRW(menu, "LandDiffuse", TW_TYPE_COLOR4F, &mLandMat.Diffuse, "group='Land Material'");
	//TwAddVarRW(menu, "LandSpecular", TW_TYPE_COLOR4F, &mLandMat.Specular, "group='Land Material'");
	//TwAddVarRW(menu, "LandSpecPower", TW_TYPE_FLOAT, &mLandMat.Specular.w, "group='Land Material' min=16.0f");
	//TwDefine("Settings/'Box Material' group='Materials'");
	//TwDefine("Settings/'Land Material' group='Materials'");
	//TwDefine("Settings/'Maze Material' group='Materials'");
	//TwDefine("Settings/Materials opened=false");

	// Tessellation
	TwAddVarRW(menu, "tess_heightScale", TW_TYPE_FLOAT, &tess_heightScale, "group='Tessellation' step=0.01f");
	TwAddVarRW(menu, "tess_maxTessDistance", TW_TYPE_FLOAT, &tess_maxTessDistance, "group='Tessellation' step=0.1f");
	TwAddVarRW(menu, "tess_minTessDistance", TW_TYPE_FLOAT, &tess_minTessDistance, "group='Tessellation' step=0.1f");
	TwAddVarRW(menu, "tess_minTessFactor", TW_TYPE_FLOAT, &tess_minTessFactor, "group='Tessellation' step=0.1f");
	TwAddVarRW(menu, "tess_maxTessFactor", TW_TYPE_FLOAT, &tess_maxTessFactor, "group='Tessellation' step=0.1f");
	TwAddVarRW(menu, "mesh_minTessFactor", TW_TYPE_FLOAT, &mesh_maxTessFactor, "group='Tessellation' step=0.1f");
	TwAddVarRW(menu, "mesh_maxTessFactor", TW_TYPE_FLOAT, &mesh_heightScale, "group='Tessellation' step=0.1f");
	TwDefine("Settings/Tessellation opened=false");

	

	// Init menu in entities
	mTerrain.buildMenu(menu);
	mSky->buildMenu(menu);
	drawManager->buildMenu(menu);
	TwAddVarRW(menu, "Camera walkmode", TW_TYPE_BOOLCPP, &lockCamera, "group=Camera");
	TwAddVarRW(menu, "Camera height", TW_TYPE_FLOAT, &mCam.height, "group=Camera");
	TwAddVarRW(menu, "Camera smooth factor", TW_TYPE_FLOAT, &mCam.smoothFactor, "group=Camera");
	mCam.buildMenu(menu);

	TwAddVarRW(menu, "Camera follow pacman", TW_TYPE_BOOLCPP, &lockPacmanCamera, "group=Game");

	TwAddSeparator(menu, NULL, NULL);
	pacman.entity->buildMenu(menu);
	pacman.maze->buildMenu(menu);
	//sound.buildMenu(menu);
}

void DXRenderer::initDX()
{
	// Create Device
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,							// use primary Dispslay adapter
		D3D_DRIVER_TYPE_HARDWARE,	// use hardware acceleration
		0,							// no software device
		createDeviceFlags,			// set debug flag
		0,							// feature level array
		0,							// feature level array size     
		D3D11_SDK_VERSION,			// SDK version
		&dxDevice,					// returns d3dDevice
		&featureLevel,				// returns feature level
		&dxDeviceContext);			// returns device context
	if(FAILED(hr))
	{
		QMessageBox::information(0, "Error", "D3D11CreateDevice Failed.");
		return;
	}
	if(featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		QMessageBox::information(0, "Error", "Direct3D Feature Level 11 unsupported.");
		return;
	}

	// Describe swap chain.
	DXGI_SWAP_CHAIN_DESC desc_sc;
	// buffer description = back buffer properties
	desc_sc.BufferDesc.Width = clientWidth;						// width
	desc_sc.BufferDesc.Height = clientHeight;					// height
	desc_sc.BufferDesc.RefreshRate.Numerator = 60;
	desc_sc.BufferDesc.RefreshRate.Denominator = 1;
	desc_sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// set pixelformat
	desc_sc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc_sc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// sample description, used to set MSAA
	HR(dxDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaa_quality));
	assert(msaa_quality > 0);
	if(msaa_enable)
	{
		desc_sc.SampleDesc.Count   = 4;
		desc_sc.SampleDesc.Quality = msaa_quality-1;
	}
	else
	{
		desc_sc.SampleDesc.Count   = 1;
		desc_sc.SampleDesc.Quality = 0;
	}
	desc_sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// set backbuffer as render target 
	desc_sc.BufferCount = 1;								// nr of backbuffers
	desc_sc.OutputWindow = winId;							// set output window
	desc_sc.Windowed = true;								// set window mode/fullscreen
	desc_sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// let display driver select most efficient presentation method
	desc_sc.Flags = 0;										// optional flags, such as DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH

	// Use IDXGIFactory to create swap chain
	IDXGIDevice* dxgiDevice = 0;
	HR(dxDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));
	// finally the IDXGIFactory interface
	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));
	// create swap chain
	HR(dxgiFactory->CreateSwapChain(dxDevice, &desc_sc, &dxSwapChain));
	// release acquired COM interfaces since we're done with them
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// Resize
	onResize(800, 600);
}

void DXRenderer::onResize(int width, int height)
{
	this->clientWidth = width;
	this->clientHeight = height;

	// Resize directx
	resizeDX();

	// Resize menu
	TwWindowSize(width, height);

	// Resize camera
	mCam.SetLens(0.25f*XM_PI, getAspectRatio(), 1.0f, 1000.0f);
}

void DXRenderer::resizeDX()
{
	// Release old views, they hold references to buffers we will be destroying; release the old depth/stencil buffer
	ReleaseCOM(view_renderTarget);
	ReleaseCOM(view_depthStencil);
	ReleaseCOM(tex_depthStencil);
	assert(dxDeviceContext);
	assert(dxDevice);
	assert(dxSwapChain);

	// Resize the swap chain
	HR(dxSwapChain->ResizeBuffers(1, clientWidth, clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Create render target view
	ID3D11Texture2D* backBuffer;
	// get pointer to backbuffer
	HR(dxSwapChain->GetBuffer(
		0,											// backbuffer index (in case there is more than one)
		__uuidof(ID3D11Texture2D),					// interface type of the buffer
		reinterpret_cast<void**>(&backBuffer)));	// returns backbuffer pointer
	// create render target view
	HR(dxDevice->CreateRenderTargetView(
		backBuffer,				// resource to be used as render target
		0,						// pointer to D3D11_RENDER_TARGET_VIEW_DESC (only needed when using typless format)
		&view_renderTarget));	// returns pointer to render target view
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC desc_depthStencil; 
	desc_depthStencil.Width     = clientWidth;						// width
	desc_depthStencil.Height    = clientHeight;						// height
	desc_depthStencil.MipLevels = 1;								// nr of mipmap levels
	desc_depthStencil.ArraySize = 1;								// nr of textures in a texture array
	desc_depthStencil.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;	// format
	// set MSSA, settings must match those in swap chain
	if(msaa_enable)
	{
		desc_depthStencil.SampleDesc.Count   = 4;
		desc_depthStencil.SampleDesc.Quality = msaa_quality-1;
	}
	else
	{
		desc_depthStencil.SampleDesc.Count   = 1;
		desc_depthStencil.SampleDesc.Quality = 0;
	}
	desc_depthStencil.Usage          = D3D11_USAGE_DEFAULT;			// specify how the depth buffer will be used
	desc_depthStencil.BindFlags      = D3D11_BIND_DEPTH_STENCIL;	// specify where resource will be bound to pipline
	desc_depthStencil.CPUAccessFlags = 0;							// specify if cpu will read resource
	desc_depthStencil.MiscFlags      = 0;							// flags; not appliable to depth buffer
	HR(dxDevice->CreateTexture2D(&desc_depthStencil, 0, &tex_depthStencil));
	HR(dxDevice->CreateDepthStencilView(tex_depthStencil, 0, &view_depthStencil));

	// Bind render target view and depth/stencil view to pipeline
	dxDeviceContext->OMSetRenderTargets(
		1,						// nr of render targets
		&view_renderTarget,		// first element of array of rendertargets
		view_depthStencil);		// pointer to depth/stencil view

	// Set viewport transform.
	viewport_screen.TopLeftX = 0;
	viewport_screen.TopLeftY = 0;
	viewport_screen.Width    = static_cast<float>(clientWidth);
	viewport_screen.Height   = static_cast<float>(clientHeight);
	viewport_screen.MinDepth = 0.0f;
	viewport_screen.MaxDepth = 1.0f;
	dxDeviceContext->RSSetViewports(
		1,								// nr of viewports
		&viewport_screen);				// viewport array
}

float DXRenderer::getAspectRatio()
{
	return (float)clientWidth/clientHeight;;
}

void DXRenderer::initGameEntities()
{
	//Init camera
	mCam.SetLens(0.25f*XM_PI, getAspectRatio(), 1.0f, 3000.0f);
	lockCamera = false;
	lockPacmanCamera = false;
}

void DXRenderer::update(float dt)
{
	// Control camera
	if(GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);
	if(GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);
	if(GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);
	if(GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);
	
	// Pacman controls
	D3DXVECTOR3 v(0.0f,0.0f,0.0f);
	if(GetAsyncKeyState(VK_LEFT) & 0x8000 )
		v.x=-1;
	if(GetAsyncKeyState(VK_RIGHT) & 0x8000 )
		v.x=1;
	if(GetAsyncKeyState(VK_UP) & 0x8000 )
		v.z=1;
	if(GetAsyncKeyState(VK_DOWN) & 0x8000 )
		v.z=-1;
	if(GetAsyncKeyState(VK_SPACE) & 0x8000 )
		pacman.entity->stop();
	int moveX = (int)(v.x);
	int moveY = (int)(v.z);
	if(moveX!=0 || moveY!=0)
	{
		pacman.entity->move(moveX, moveY);
	}

	//
	// Gameloop
	//

	// Sound
	//sound.update(dt);

	//Update camera
	if(lockCamera)
	{
		XMFLOAT3 oldPos = mCam.GetPosition();
		XMFLOAT3 newPos = oldPos; newPos.y = mTerrain.getTerrainHeight(oldPos.x, oldPos.z) + mCam.height;

		XMFLOAT3 interpolatet_pos; XMStoreFloat3(&interpolatet_pos, XMVectorLerp( XMLoadFloat3(&oldPos),  XMLoadFloat3(&newPos), dt*mCam.smoothFactor));
		mCam.SetPosition(interpolatet_pos);
	}
	if(lockPacmanCamera)
	{
		XMMATRIX pacPos = pacman.entity->getPos();
		XMVECTOR oldPos = mCam.GetPositionXM();
		XMVECTOR newPos = XMVectorSet(pacPos._41, XMVectorGetY(oldPos), pacPos._43, 1.0f);
		XMFLOAT3 interpolatet_pos;
		XMStoreFloat3(&interpolatet_pos,XMVectorLerp(oldPos, newPos, dt*2.0f));
		mCam.SetPosition(interpolatet_pos);
	}

	// Update game
	pacman.run(dt);

	drawManager->buildShadowTransform();
	mCam.UpdateViewMatrix();

	
}

void DXRenderer::renderFrame()
{
	mSmap->BindDsvAndSetNullRenderTarget(dxDeviceContext);
	DrawSceneToShadowMap();
	dxDeviceContext->RSSetState(0);

	// Restore the back and depth buffer to the OM stage.
	ID3D11RenderTargetView* renderTargets[1] = {view_renderTarget};
	dxDeviceContext->OMSetRenderTargets(1, renderTargets, view_depthStencil);
	dxDeviceContext->RSSetViewports(1, &viewport_screen);

	// Clear render target & depth/stencil
	dxDeviceContext->ClearRenderTargetView(view_renderTarget, reinterpret_cast<const float*>(&Colors::DeepBlue));
	dxDeviceContext->ClearDepthStencilView(view_depthStencil, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set per frame constants.
	FXStandard* fx = shaderManager->effects.fx_standard;
	fx->SetEyePosW(mCam.GetPosition());
	fx->SetCubeMap(mSky->CubeMapSRV());
	fx->SetShadowMap(mSmap->DepthMapSRV());

	// Tessellation settings
	fx->SetHeightScale(tess_heightScale);
	fx->SetMaxTessDistance(tess_maxTessDistance);
	fx->SetMinTessDistance(tess_minTessDistance);
	fx->SetMinTessFactor(tess_minTessFactor);
	fx->SetMaxTessFactor(tess_maxTessFactor);

	dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	//dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	drawManager->prepareFrame();

	if(wireframe_enable)
		dxDeviceContext->RSSetState(shaderManager->states.WireframeRS);


	//
	// Draw
	// 

	drawGame();
	
	
	if(drawTerrain)
		mTerrain.draw(dxDeviceContext, &mCam);

	dxDeviceContext->RSSetState(0);

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	dxDeviceContext->HSSetShader(0, 0, 0);
	dxDeviceContext->DSSetShader(0, 0, 0);
	dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Debug view depth buffer.
	if(GetAsyncKeyState('Z') & 0x8000)
	{
		DrawScreenQuad(mSmap->DepthMapSRV());
	}

	if(drawSky)
		mSky->Draw(dxDeviceContext, &mCam);

	// restore default states, as the SkyFX changes them in the effect file.
	dxDeviceContext->RSSetState(0);
	dxDeviceContext->OMSetDepthStencilState(0, 0);

	// Unbind shadow map as a shader input because we are going to render to it next frame.
	// The shadow might be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	dxDeviceContext->PSSetShaderResources(0, 16, nullSRV);

	// Draw menu
	TwDraw(); 

	// Show the finished frame
	HR(dxSwapChain->Present(0, 0));
}

void DXRenderer::drawGame()
{
	FXStandard* fx = shaderManager->effects.fx_standard;
	XMMATRIX viewProj = mCam.ViewProj();

	ID3DX11EffectTechnique* tech = fx->tech_tess;
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for(UINT pass = 0; pass<techDesc.Passes; pass++)
	{
		// Draw plane
		if(drawPlane)
		{
			drawManager->drawObject(1, 0, XMMatrixIdentity(), viewProj, pass);
		}

		// Tessellation settings
		fx->SetHeightScale(mesh_heightScale);
		fx->SetMaxTessDistance(5.0);
		fx->SetMinTessDistance(100.0);
		fx->SetMinTessFactor(1.0);
		fx->SetMaxTessFactor(mesh_maxTessFactor);


		if(drawPacman)
		{
			

			// Draw maze
			for(int y = 0; y<pacman.maze->getSizeY(); y++)
			{
				for(int x = 0; x<pacman.maze->getSizeX(); x++)
				{
					XMMATRIX world = (XMMATRIX)pacman.maze->getPosition(x,y);
					XMMATRIX scale = XMMatrixScalingFromVector(XMVectorReplicate(1.0f));

					if(pacman.maze->getTile(x,y)==1)
					{
						drawManager->drawObject(0, 1, scale*world, viewProj, pass);
					}
					if(pacman.maze->getTile(x,y)==0)
					{
						scale = XMMatrixScalingFromVector(XMVectorReplicate(0.1f));
						drawManager->drawObject(0, 1, scale*world, viewProj, pass);
					}
				}
			}

			// Draw game entities
			XMMATRIX world = (XMMATRIX)pacman.entity->getPos();
			XMMATRIX scale = XMMatrixScalingFromVector(XMVectorReplicate(0.7f));
			drawManager->drawObject(0, 2, scale*world, viewProj, pass);
		}

		// Draw mesh
		if(drawMesh)
		{
			dxDeviceContext->RSSetState(shaderManager->states.NoCullRS);
			if(wireframe_enable)
				dxDeviceContext->RSSetState(shaderManager->states.WireframeRS);
			drawManager->drawMesh(0, viewProj, pass);
			dxDeviceContext->RSSetState(0);
		}
	}

	//
	// Instancing
	//

	//// Draw pacman 
	//if(drawPacman)
	//{

	//	UINT stride[2] = {sizeof(Vertex::posNormTexTan), sizeof(Vertex::InstancedData)};

	//	tech = fx->tech_tess_inst;
	//	tech->GetDesc(&techDesc);
	//	for(UINT pass = 0; pass<techDesc.Passes; pass++)
	//	{
	//		// Draw maze -- with instancing!
	//		// mapp world matrices to instance buffer
	//		num_visibleObjects = 0;
	//		D3D11_MAPPED_SUBRESOURCE mappedData; 
	//		dxDeviceContext->Map(instancedBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedData);
	//		Vertex::InstancedData* data = reinterpret_cast<Vertex::InstancedData*>(mappedData.pData);
	//		static float timer = 1.0f;
	//		timer += 0.001;
	//		XMStoreFloat4x4(&data[10].World, XMMatrixTranslation(8.0f,5.0f,100.0f));
	//		/*	for(int y = 0; y<pacman.maze->getSizeY(); y++)
	//		{
	//		for(int x = 0; x<pacman.maze->getSizeX(); x++)
	//		{
	//		XMMATRIX world = (XMMATRIX)pacman.maze->getPosition(x,y);
	//		XMMATRIX scale = XMMatrixScalingFromVector(XMVectorReplicate(2.0f));
	//		if(pacman.maze->getTile(x,y)==1)
	//		{
	//		XMFLOAT4X4 inst_World; XMStoreFloat4x4(&inst_World, scale*world);
	//		data[num_visibleObjects].World = inst_World;
	//		num_visibleObjects++;
	//		}
	//		if(pacman.maze->getTile(x,y)==0)
	//		{
	//		scale = XMMatrixScalingFromVector(XMVectorReplicate(0.1f));
	//		XMFLOAT4X4 inst_World; XMStoreFloat4x4(&inst_World, scale*world);
	//		data[num_visibleObjects].World = inst_World;
	//		num_visibleObjects++;
	//		}
	//		}
	//		}*/
	//		dxDeviceContext->Unmap(instancedBuffer, 0);
	//		// draw instance buffer
	//		drawManager->prepareFrameInstanced(stride, instancedBuffer);
	//		// Set input layout, topology, context
	//		dxDeviceContext->IASetInputLayout(shaderManager->layout_inst_posNormTexTan);

	//		drawManager->drawObjectInstanced(0, 2, pacman.maze->getSizeY(), viewProj, pass);
	//	}
	//}
}

void DXRenderer::DrawSceneToShadowMap()
{
	XMMATRIX view     = XMLoadFloat4x4(&drawManager->mLightView);
	XMMATRIX proj     = XMLoadFloat4x4(&drawManager->mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	FXBuildShadowMap* fx = shaderManager->effects.fx_buildShadowMap;
	fx->SetEyePosW(mCam.GetPosition());
	fx->SetViewProj(viewProj);

	// These properties could be set per object if needed.
	fx->SetHeightScale(tess_heightScale);
	fx->SetMaxTessDistance(tess_maxTessDistance);
	fx->SetMinTessDistance(tess_minTessDistance);
	fx->SetMinTessFactor(tess_minTessFactor);
	fx->SetMaxTessFactor(tess_maxTessFactor);

	drawManager->prepareFrame_shadowMap();

	// Draw
	ID3DX11EffectTechnique* tech = fx->TessBuildShadowMapTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for(UINT pass = 0; pass < techDesc.Passes; pass++)
	{
		// Draw land
		if(drawPlane)
			drawManager->drawObject_shadowMap(1, 0, XMMatrixIdentity(), viewProj, pass);

		// Tessellation settings
		fx->SetHeightScale(mesh_heightScale);
		fx->SetMaxTessDistance(5.0);
		fx->SetMinTessDistance(100.0);
		fx->SetMinTessFactor(1.0);
		fx->SetMaxTessFactor(mesh_maxTessFactor);


		// Draw mesh
		if(drawMesh)
		{
			drawManager->drawMesh_shadowMap(0, viewProj, pass);
		}

		// Draw pacman
		if(drawPacman)
		{
			fx->SetHeightScale(0.0f);
			// Draw maze
			for(int y = 0; y<pacman.maze->getSizeY(); y++)
			{
				for(int x = 0; x<pacman.maze->getSizeX(); x++)
				{
					XMMATRIX world = (XMMATRIX)pacman.maze->getPosition(x,y);
					XMMATRIX scale = XMMatrixScalingFromVector(XMVectorReplicate(1.0f));

					if(pacman.maze->getTile(x,y)==1)
					{
						drawManager->drawObject_shadowMap(0, 1, scale*world, viewProj, pass);
					}
					if(pacman.maze->getTile(x,y)==0)
					{
						scale = XMMatrixScalingFromVector(XMVectorReplicate(0.1f));
						drawManager->drawObject_shadowMap(0, 1, scale*world, viewProj, pass);
					}
				}
			}

			// Draw game entities
			XMMATRIX world = (XMMATRIX)pacman.entity->getPos();
			XMMATRIX scale = XMMatrixScalingFromVector(XMVectorReplicate(0.7f));
			drawManager->drawObject_shadowMap(0, 2, scale*world, viewProj, pass);
		}
	}
}

void DXRenderer::DrawScreenQuad(ID3D11ShaderResourceView* resource)
{
	UINT stride = sizeof(Vertex::posNormTex);
	UINT offset = 0;

	dxDeviceContext->IASetInputLayout(shaderManager->layout_posNormTex);
	dxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxDeviceContext->IASetVertexBuffers(0, 1, &drawManager->mScreenQuadVB, &stride, &offset);
	dxDeviceContext->IASetIndexBuffer(drawManager->mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	FXShowTexture* fx = shaderManager->effects.fx_showTexture;
	ID3DX11EffectTechnique* tech = fx->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		fx->SetWorldViewProj(world);
		fx->SetTexture(resource);

		tech->GetPassByIndex(p)->Apply(0, dxDeviceContext);
		dxDeviceContext->DrawIndexed(6, 0, 0);
	}
}

void DXRenderer::initInstanceBuffer()
{
	std::vector<Vertex::InstancedData> instances(pacman.maze->getSizeY() * pacman.maze->getSizeX());
	
	for(int i = 0; i<(int)instances.size(); i++)
	{
		XMFLOAT4X4 identity; XMStoreFloat4x4(&identity, XMMatrixIdentity());//XMMatrixTranslation(i,i,i)
		instances[i].World = identity;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::InstancedData) * instances.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	HR(dxDevice->CreateBuffer(&vbd, 0, &instancedBuffer));
}
