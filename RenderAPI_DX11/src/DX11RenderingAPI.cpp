#include "DX11RenderingAPI.h"
#include "../../RamJamEngine/include/System.h"


DX11RenderingAPI::~DX11RenderingAPI() {}

//////////////////////////////////////////////////////////////////////////
DX11RenderingAPI::DX11RenderingAPI(Scene& scene) : mScene(scene)
{
	mDX11Device			= nullptr;
	mDX11DepthBuffer	= nullptr;
	mSwapChain			= nullptr;
	mRenderTargetView	= nullptr;
	md3dDriverType		= D3D_DRIVER_TYPE_HARDWARE;
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));
	//-----------
	VSyncEnabled = true;
	//-----------
	mTextureMgr   = nullptr;
	mConsoleFont  = nullptr;
	mProfilerFont = nullptr;
	mSpriteBatch  = nullptr;
	//-----------
	mVertexBuffer = nullptr;
	mIndexBuffer  = nullptr;
	mVertexBuffer_Gizmo = nullptr;
	mIndexBuffer_Gizmo  = nullptr;
	//-----------
	mBoxMap            = nullptr;
	mGridMap           = nullptr;
	mSphereMap         = nullptr;
	mCylinderMap       = nullptr;
	mMaskMap           = nullptr;
	mWhiteSRV          = nullptr;
	mConsoleBackground = nullptr;
	mRjeLogo           = nullptr;
	//-----------
	mEyePosW = Vector3::zero;
	//-----------
	mBlendFactorR = 0.5f;
	mBlendFactorG = 0.5f;
	mBlendFactorB = 0.5f;
	mBlendFactorA = 1.0f;
	//-----------
	mDirLightCount    = 1;

	// Texture transforms
	mGridTexTransform = Matrix44::Scaling(30.0f, 30.0f, 0.0f);
	mBoxTexTransform  = mCylinderTexTransform = mSphereTexTransform = Matrix44::Scaling(1.0f, 1.0f, 0.0f);

	// Meshes transforms
	mModelRot = TwQuaternion();
	mGridWorld = Matrix44::identity;
	Transform tempTrf;
// 	tempTrf.Rotation	= Quaternion(45,45,45).ToMatrix();
// 	tempTrf.Scale		= Vector3(2.5f, 5.0f, 0.5f);
	tempTrf.Position	= Vector3(0.0f, 5.0f, 0.0f);
	mWireBoxWorld		= mAxisWorld = tempTrf.WorldMatrix();
	//-------
	tempTrf.Scale		= Vector3(2.0f, 1.0f, 2.0f);
	tempTrf.Position	= Vector3(0.0f, 0.51f, 0.0f);		//0.51 to avoid z-fight occurring with 0.5
	mBoxWorld			= tempTrf.WorldMatrix();
	//-------
	tempTrf.Rotation	= Quaternion(0,45,0).ToMatrix();
	tempTrf.Scale		= Vector3(0.2f, 0.2f, 0.2f);
	tempTrf.Position	= Vector3(0.0f, 1.0f, 0.0f);
	mModelWorld			= tempTrf.WorldMatrix();
	//-------
	for(int i = 0; i < 5; ++i)
	{
		mCylWorld[i*2+0] = Matrix44::Translation(-5.0f, 1.5f, -10.0f + i*5.0f);
		mCylWorld[i*2+1] = Matrix44::Translation(+5.0f, 1.5f, -10.0f + i*5.0f);
		//-------
		mSphereWorld[i*2+0] = Matrix44::Translation(-5.0f, 3.5f, -10.0f + i*5.0f);
		mSphereWorld[i*2+1] = Matrix44::Translation(+5.0f, 3.5f, -10.0f + i*5.0f);
	}

	// Light Specs
	mPointLights = nullptr;
	Vector4 black = Color(Color::Black).GetVector4RGBANorm();
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		mWorkingPointLights[i].Diffuse  = Color::GetRandomRGBNorm();
		mWorkingPointLights[i].Ambient  = black;
		mWorkingPointLights[i].Specular = Vector4(0.7f, 0.7f, 0.7f, 1.0f);
		mWorkingPointLights[i].Att      = Vector3(0.175f, 0.175f, 0.175f);
		mWorkingPointLights[i].Range    = 50.0f;
	}
	mDirLights[0].Ambient   = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse   = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular  = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = Vector3(0.57735f, -0.57735f, 0.57735f);
	//--------
	mDirLights[1].Ambient   = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse   = Vector4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular  = Vector4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = Vector3(-0.57735f, -0.57735f, 0.57735f);
	//--------
	mDirLights[2].Ambient   = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse   = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular  = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = Vector3(0.0f, -0.707f, -0.707f);

	// Material Specs
	mGridMat.AddPropertyVector("Ambient",  Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	mGridMat.AddPropertyVector("Diffuse",  Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	mGridMat.AddPropertyVector("Specular", Vector4(0.2f, 0.2f, 0.2f, 16.0f));
	mGridMat.AddPropertyVector("Reflect",  Vector4::zero);
	//--------
	mCylinderMat.AddPropertyVector("Ambient",  Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	mCylinderMat.AddPropertyVector("Diffuse",  Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	mCylinderMat.AddPropertyVector("Specular", Vector4(0.2f, 0.2f, 0.2f, 16.0f));
	mCylinderMat.AddPropertyVector("Reflect",  Vector4::zero);
	//--------
	mSphereMat.AddPropertyVector("Ambient",  Vector4(0.6f, 0.8f, 0.9f, 1.0f));
	mSphereMat.AddPropertyVector("Diffuse",  Vector4(0.6f, 0.8f, 0.9f, 1.0f));
	mSphereMat.AddPropertyVector("Specular", Vector4(0.9f, 0.9f, 0.9f, 16.0f));
	mSphereMat.AddPropertyVector("Reflect",  Vector4::zero);
	//--------
	mBoxMat.AddPropertyVector("Ambient",  Vector4(0.5f, 0.5f, 0.5f, 1.0f));
	mBoxMat.AddPropertyVector("Diffuse",  Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	mBoxMat.AddPropertyVector("Specular", Vector4(0.2f, 0.2f, 0.2f, 16.0f));
	mBoxMat.AddPropertyVector("Reflect",  Vector4::zero);
	//--------
	mModelMat.AddPropertyVector("Ambient",  Vector4(0.8f, 0.8f, 0.8f, 1.0f));
	mModelMat.AddPropertyVector("Diffuse",  Vector4(0.8f, 0.8f, 0.8f, 1.0f));
	mModelMat.AddPropertyVector("Specular", Vector4(0.8f, 0.8f, 0.8f, 16.0f));
	mModelMat.AddPropertyVector("Reflect",  Vector4::zero);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::Initialize(int windowWidth, int windowHeight)
{
	mDX11Device       = rje_new DX11Device;
	mDX11DepthBuffer  = rje_new DX11DepthBuffer;
	mDX11CommonStates = rje_new DX11CommonStates;

	mWindowWidth  = windowWidth;
	mWindowHeight = windowHeight;

	// Create the device and device context.
	UINT createDeviceFlags = 0;
#ifdef RJE_DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	RJE_CHECK_FOR_SUCCESS( D3D11CreateDevice(	0,					// default adapter
												md3dDriverType,
												0,					// no software device
												createDeviceFlags,
												0, 0,				// default feature level array
												D3D11_SDK_VERSION,
												&mDX11Device->md3dDevice,
												&featureLevel,
												&mDX11Device->md3dImmediateContext));

	if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		RJE_MESSAGE_BOX(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return;
	}

	//////////////////////////////////////////////////////////////////////////

	PROFILE_GPU_INIT(mDX11Device->md3dDevice, mDX11Device->md3dImmediateContext);

	//////////////////////////////////////////////////////////////////////////
	
	//------- Not used for now
// #ifdef RJE_DEBUG
// 	if( SUCCEEDED( mDX11Device->md3dDevice->QueryInterface( __uuidof(ID3D11Debug), (void**)&md3dDebug ) ) )
// 	{
// 		if( SUCCEEDED( md3dDebug->QueryInterface( __uuidof(ID3D11InfoQueue), (void**)&md3dInfoQueue ) ) )
// 		{
// 			md3dInfoQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_CORRUPTION, true );
// 			md3dInfoQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_ERROR, true );
// 
// 			D3D11_MESSAGE_ID hide [] =
// 			{
// 				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
// 				// Add more message IDs here as needed
// 			};
// 
// 			D3D11_INFO_QUEUE_FILTER filter;
// 			memset( &filter, 0, sizeof(filter) );
// 			filter.DenyList.NumIDs = _countof(hide);
// 			filter.DenyList.pIDList = hide;
// 			md3dInfoQueue->AddStorageFilterEntries( &filter );
// 			md3dInfoQueue->Release();
// 		}
// 		md3dDebug->Release();
// 	}
// #endif

	//--------------------
	InitSwapChain();
	//--------------------

	IDXGIDevice*  dxgiDevice  = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;

	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	RJE_CHECK_FOR_SUCCESS(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	DXGI_ADAPTER_DESC adapterDesc;
	dxgiAdapter->GetDesc(&adapterDesc);
	memcpy_s(System::Instance()->mGpuDescription, 128*sizeof(WCHAR), adapterDesc.Description, 128*sizeof(WCHAR));
	// Remove white space at the end
	for(size_t i=wcslen(System::Instance()->mGpuDescription)-1 ; i>0 ; --i)
	{
		if(System::Instance()->mGpuDescription[i] == ' ')
			System::Instance()->mGpuDescription[i] = nullchar;
		else break;
	}
	System::Instance()->mGpuDedicatedVRAM = (adapterDesc.DedicatedVideoMemory/1024)/1024;
	System::Instance()->mGpuSharedVRAM    = (adapterDesc.SharedSystemMemory/1024)/1024;

	RJE_SAFE_RELEASE(dxgiDevice);
	RJE_SAFE_RELEASE(dxgiAdapter);

	//---------------------
	ResizeWindow();	// we call the ResizeWindow method here to avoid code duplication.
	//---------------------

	// We init the effect first and then the input layouts
	DX11Effects     ::InitAll(mDX11Device->md3dDevice);
	DX11InputLayouts::InitAll(mDX11Device->md3dDevice);
	DX11CommonStates::InitAll(mDX11Device->md3dDevice);

	mTextureMgr = rje_new DX11TextureManager;
	mTextureMgr->Initialize(mDX11Device->md3dDevice);

	// Init the 2d elements
	mProfilerFont = rje_new DX11FontSheet();
	mConsoleFont  = rje_new DX11FontSheet();
	mSpriteBatch  = rje_new DX11SpriteBatch();
	RJE_CHECK_FOR_SUCCESS(mConsoleFont-> Initialize(mDX11Device->md3dDevice, L"Consolas", 16.0f, FontSheet::FontStyleRegular, true));
	RJE_CHECK_FOR_SUCCESS(mProfilerFont->Initialize(mDX11Device->md3dDevice, L"Consolas", 16.0f, FontSheet::FontStyleBold,    true));
	RJE_CHECK_FOR_SUCCESS(mSpriteBatch-> Initialize(mDX11Device->md3dDevice, mDX11Device->md3dImmediateContext));

	mTextureMgr->LoadTexture("box",      &mBoxMap);
	mTextureMgr->LoadTexture("grid",     &mGridMap);
	mTextureMgr->LoadTexture("sphere",   &mSphereMap);
	mTextureMgr->LoadTexture("cylinder", &mCylinderMap);
	mTextureMgr->LoadTexture("mask",     &mMaskMap);
	mTextureMgr->LoadTexture("rje_logo", &mRjeLogo);
	mTextureMgr->Create2DTextureFixedColor(1, RJE_COLOR::Color::TransDarkGray, &mConsoleBackground);
	mTextureMgr->Create2DTextureFixedColor(1, RJE_COLOR::Color::White,         &mWhiteSRV);
	//----------
	mBoxMat.     AddPropertyTexture("Texture_Diffuse", mBoxMap);
	mCylinderMat.AddPropertyTexture("Texture_Diffuse", mCylinderMap);
	mGridMat.    AddPropertyTexture("Texture_Diffuse", mGridMap);
	mCylinderMat.AddPropertyTexture("Texture_Diffuse", mCylinderMap);
	mSphereMat.  AddPropertyTexture("Texture_Diffuse", mSphereMap);
	mModelMat.   AddPropertyTexture("Texture_Diffuse", mWhiteSRV);
	//----------
	BuildGeometryBuffers();
	BuildGizmosBuffers();

	SetActivePointLights(3);

	//////////////////////////////////////////////////////////////////////////
	// AntTweak Bars
	if (!TwInit(TW_DIRECT3D11, mDX11Device->md3dDevice))
		MessageBoxA(System::Instance()->mHWnd, TwGetLastError(), "AntTweakBar initialization failed", MB_OK|MB_ICONERROR);
	//-----------
	// Create a tweak bar
	TwBar *bar = TwNewBar("Scene");
	TwDefine("Scene iconified=true ");
	int barSize[2] = {224, 320};
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwAddVarRW(bar, "V-Sync Enabled", TW_TYPE_BOOLCPP, &VSyncEnabled, NULL);
	TwAddSeparator(bar, NULL, NULL);
	TwAddVarRW(bar, "Draw Reflections", TW_TYPE_BOOLCPP, &mScene.mbDrawReflections, NULL);
	TwAddVarRW(bar, "Use Texture",      TW_TYPE_BOOLCPP, &mScene.mbUseTexture, NULL);
	TwAddVarRW(bar, "Use Blending",     TW_TYPE_BOOLCPP, &mScene.mbUseBlending, NULL);
	TwAddSeparator(bar, NULL, NULL);
	TwAddVarRW(bar, "Use Fog", TW_TYPE_BOOLCPP,   &mScene.mbUseFog, NULL);
	TwAddVarRW(bar, "Fog Color", TW_TYPE_COLOR4F, &mScene.mFogColor, NULL);
	TwAddVarRW(bar, "Fog Start", TW_TYPE_FLOAT,   &mScene.mFogStart, "min=10 max=100");
	TwAddVarRW(bar, "Fog Range", TW_TYPE_FLOAT,   &mScene.mFogRange, "min=20 max=500");
	TwAddSeparator(bar, NULL, NULL);
	TwAddVarRW(bar, "Blend Factor Red",   TW_TYPE_FLOAT, &mBlendFactorR, "min=0 max=1 step=0.05");
	TwAddVarRW(bar, "Blend Factor Green", TW_TYPE_FLOAT, &mBlendFactorG, "min=0 max=1 step=0.05");
	TwAddVarRW(bar, "Blend Factor Blue",  TW_TYPE_FLOAT, &mBlendFactorB, "min=0 max=1 step=0.05");
	TwAddVarRW(bar, "Blend Factor Alpha", TW_TYPE_FLOAT, &mBlendFactorA, "min=0 max=1 step=0.05");
	TwAddSeparator(bar, NULL, NULL);
	TwAddButton(bar, "Toggle Wireframe", TwSetWireframe, this, NULL);
	TwAddSeparator(bar, NULL, NULL);
	TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &mModelRot, "opened=true");
	//-----------
	TwBar *camBar = TwNewBar("Camera");
	TwDefine("Camera iconified=true ");
	TwSetParam(camBar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwAddVarRW(camBar, "Position", TW_TYPE_DIR3F, &mCamera->mPosition, "opened=false");
	TwAddVarRW(camBar, "LookAt", TW_TYPE_DIR3F, &mCamera->mLookAt, "opened=false");
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::InitSwapChain(UINT msaaSamples)
{
	HWND hMainWnd = System::Instance()->mHWnd;
	MSAA_Samples = msaaSamples;

	RJE_SAFE_RELEASE(mSwapChain);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width						= mWindowWidth;
	sd.BufferDesc.Height					= mWindowHeight;
	sd.BufferDesc.RefreshRate.Numerator		= 60;
	sd.BufferDesc.RefreshRate.Denominator	= 1;
	sd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	sd.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count						= MSAA_Samples;
	sd.SampleDesc.Quality					= 0;

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = hMainWnd;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain:
	// This function is being called with a device from a different IDXGIFactory."
	IDXGIDevice*  dxgiDevice  = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;
	IDXGIFactory* dxgiFactory = nullptr;

	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	RJE_CHECK_FOR_SUCCESS(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));
	RJE_CHECK_FOR_SUCCESS(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));
	RJE_CHECK_FOR_SUCCESS(dxgiFactory->CreateSwapChain(mDX11Device->md3dDevice, &sd, &mSwapChain));
	RJE_CHECK_FOR_SUCCESS(dxgiFactory->MakeWindowAssociation(hMainWnd, DXGI_MWA_NO_WINDOW_CHANGES));

	RJE_SAFE_RELEASE(dxgiDevice);
	RJE_SAFE_RELEASE(dxgiAdapter);
	RJE_SAFE_RELEASE(dxgiFactory);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::BuildGeometryBuffers()
{
	FILE* fIn;
	string modelPath = System::Instance()->mDataPath + CIniFile::GetValue("modelpath", "meshes", System::Instance()->mResourcesPath);
	fopen_s(&fIn, modelPath.c_str(), "rb");

	if(!fIn)
	{
		RJE_MESSAGE_BOX(0, L"dragon.mesh not found.", 0, 0);
		return;
	}

	UINT modelVertexCount = 0;
	UINT modelTriangleCount = 0;

	fread(&modelVertexCount,   sizeof(UINT), 1, fIn);
	fread(&modelTriangleCount, sizeof(UINT), 1, fIn);

	mModelIndexCount = 3*modelTriangleCount;

	MeshData::Data<PosNormTanTex> box;
	MeshData::Data<PosNormTanTex> grid;
	MeshData::Data<PosNormTanTex> sphere;
	MeshData::Data<PosNormTanTex> cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(100.0f, 100.0f, 2, 2, grid);
	//geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateGeosphere(0.5f, 3, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset      = 0;
	mGridVertexOffset     = (int) box.Vertices.size();
	mSphereVertexOffset   = (int) (mGridVertexOffset + grid.Vertices.size());
	mCylinderVertexOffset = (int) (mSphereVertexOffset + sphere.Vertices.size());
	mModelVertexOffset    = (int) (mCylinderVertexOffset + cylinder.Vertices.size());

	// Cache the index count of each object.
	mBoxIndexCount      = (UINT) box.Indices.size();
	mGridIndexCount     = (UINT) grid.Indices.size();
	mSphereIndexCount   = (UINT) sphere.Indices.size();
	mCylinderIndexCount = (UINT) cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset      = 0;
	mGridIndexOffset     = mBoxIndexCount;
	mSphereIndexOffset   = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;
	mModelIndexOffset    = mCylinderIndexOffset + mCylinderIndexCount;

	UINT totalVertexCount = (UINT) box.Vertices.size() +
							(UINT) grid.Vertices.size() +
							(UINT) sphere.Vertices.size() +
							(UINT) cylinder.Vertices.size() +
							modelVertexCount;

	UINT totalIndexCount =  mBoxIndexCount +
							mGridIndexCount + 
							mSphereIndexCount +
							mCylinderIndexCount +
							mModelIndexCount;


	std::vector<MeshData::PosNormalTex> vertices(totalVertexCount);
	UINT k = 0;

	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex    = box.Vertices[i].TexC;
	}
	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex    = grid.Vertices[i].TexC;
	}
	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex    = sphere.Vertices[i].TexC;
	}
	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex    = cylinder.Vertices[i].TexC;
	}
	for(size_t i = 0; i < modelVertexCount; ++i, ++k)
	{
		fread(&vertices[k].Pos.x,    sizeof(float), 1, fIn);
		fread(&vertices[k].Pos.y,    sizeof(float), 1, fIn);
		fread(&vertices[k].Pos.z,    sizeof(float), 1, fIn);
		fread(&vertices[k].Tex.x,    sizeof(float), 1, fIn);
		fread(&vertices[k].Tex.y,    sizeof(float), 1, fIn);
		fread(&vertices[k].Normal.x, sizeof(float), 1, fIn);
		fread(&vertices[k].Normal.y, sizeof(float), 1, fIn);
		fread(&vertices[k].Normal.z, sizeof(float), 1, fIn);
	}
	std::vector<UINT> dragonIndices(mModelIndexCount);
	for(UINT i = 0; i < modelTriangleCount; ++i)
	{
		fread(&dragonIndices[i*3+0], sizeof(UINT), 1, fIn);
		fread(&dragonIndices[i*3+1], sizeof(UINT), 1, fIn);
		fread(&dragonIndices[i*3+2], sizeof(UINT), 1, fIn);
	}

	fclose(fIn);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage          = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth      = (UINT) (sizeof(MeshData::PosNormalTex) * totalVertexCount);
	vbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags      = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());
	indices.insert(indices.end(), dragonIndices.begin(), dragonIndices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage          = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth      = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags      = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::BuildGizmosBuffers()
{
	MeshData::Data<ColorVertex> wireBox;
	MeshData::Data<ColorVertex> axis;

	GeometryGenerator geoGen;
	geoGen.CreateWireBox(1.0f,1.0f,1.0f, wireBox, Color::Red);
	geoGen.CreateWireSphere(0.5f, axis, Color::Green);

	UINT totalVertexCount = (UINT) wireBox.Vertices.size() + (UINT) axis.Vertices.size();
	UINT totalIndexCount  = (UINT) wireBox.Indices.size()  + (UINT) axis.Indices.size();

	MeshData::Data<ColorVertex> totalGeometry(totalVertexCount);
	UINT k = 0;
	for(size_t i = 0; i < wireBox.Vertices.size(); ++i, ++k)
	{
		totalGeometry.Vertices[k].pos   = wireBox.Vertices[i].pos;
		totalGeometry.Vertices[k].color = wireBox.Vertices[i].color;
	}
	for(size_t i = 0; i < axis.Vertices.size(); ++i, ++k)
	{
		totalGeometry.Vertices[k].pos   = axis.Vertices[i].pos;
		totalGeometry.Vertices[k].color = axis.Vertices[i].color;
	}
	totalGeometry.Indices.insert(totalGeometry.Indices.end(), wireBox.Indices.begin(), wireBox.Indices.end());
	totalGeometry.Indices.insert(totalGeometry.Indices.end(), axis.Indices.begin(), axis.Indices.end());

	mWireBoxVertexOffset = 0;
	mWireBoxIndexOffset  = 0;
	mWireBoxIndexCount   = (UINT)wireBox.Indices.size();

	mAxisVertexOffset = (int) wireBox.Vertices.size();
	mAxisIndexOffset  = mWireBoxIndexCount;
	mAxisIndexCount   = (UINT)axis.Indices.size();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth           = (UINT) (sizeof(MeshData::ColorVertex) * totalVertexCount);
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = 0;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &totalGeometry.Vertices[0];
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer_Gizmo));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &totalGeometry.Indices[0];
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer_Gizmo));
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::UpdateScene( float dt )
{
	//-------------------------------------
	Transform tempTrf;
	tempTrf.Rotation	= mModelRot;
	tempTrf.Scale		= Vector3(0.2f, 0.2f, 0.2f);
	tempTrf.Position	= Vector3(0.0f, 1.0f, 0.0f);
	mModelWorld			= tempTrf.WorldMatrix();
	//-------------------------------------

	// Convert Spherical to Cartesian coordinates.
	float x = System::Instance()->mCameraRadius*sinf(System::Instance()->mCameraPhi)*cosf(System::Instance()->mCameraTheta);
	float z = System::Instance()->mCameraRadius*sinf(System::Instance()->mCameraPhi)*sinf(System::Instance()->mCameraTheta);
	float y = System::Instance()->mCameraRadius*cosf(System::Instance()->mCameraPhi);

	mEyePosW = Vector3(x, y, z);

	// Build the view matrix.
	mCamera->mPosition = Vector3(x, y, z);
	mCamera->mLookAt   = Vector3::zero;
	mCamera->mUp       = Vector3(0.0f, 1.0f, 0.0f);
	mCamera->UpdateViewMatrix();
	
	// Inputs modifiers : TODO: Get these out of DX11RenderingAPI !
	if (!Console::Instance()->IsActive())
	{
		if (Input::Instance()->GetKeyboardDown(Keyboard0))	mDirLightCount = 0;
		if (Input::Instance()->GetKeyboardDown(Keyboard1))	mDirLightCount = 1;
		if (Input::Instance()->GetKeyboardDown(Keyboard2))	mDirLightCount = 2;
		if (Input::Instance()->GetKeyboardDown(Keyboard3))	mDirLightCount = 3;
		if (Input::Instance()->GetKeyboardDown(Add))		if (mPointLightCount < MAX_LIGHTS)	SetActivePointLights(mPointLightCount + 1);
		if (Input::Instance()->GetKeyboardDown(Subtract))	if (mPointLightCount > 0)			SetActivePointLights(mPointLightCount - 1);
		if (Input::Instance()->GetKeyboardDown(S))			{DX11CommonStates::sCurrentRasterizerState = DX11CommonStates::sRasterizerState_Solid;}
		if (Input::Instance()->GetKeyboardDown(A))			DX11CommonStates::sCurrentSamplerState    = DX11CommonStates::sSamplerState_Anisotropic;
		if (Input::Instance()->GetKeyboardDown(L))			DX11CommonStates::sCurrentSamplerState    = DX11CommonStates::sSamplerState_Linear;
		if (Input::Instance()->GetKeyboardDown(U))			DX11CommonStates::sCurrentBlendState      = DX11CommonStates::sBlendState_BlendFactor;
		if (Input::Instance()->GetKeyboardDown(I))			DX11CommonStates::sCurrentBlendState      = DX11CommonStates::sBlendState_AlphaToCoverage;
		if (Input::Instance()->GetKeyboardDown(O))			DX11CommonStates::sCurrentBlendState      = DX11CommonStates::sBlendState_Transparent;
		if (Input::Instance()->GetKeyboardDown(P))			DX11CommonStates::sCurrentBlendState      = DX11CommonStates::sBlendState_Opaque;
	}

	static float timer = 0.0f;
	timer += dt*0.5f;

	// Copy light list into shader buffer
	if (mPointLightCount > 0)
	{
		PointLight* light = mPointLights->MapDiscard(mDX11Device->md3dImmediateContext);
		for (UINT i = 0; i < mPointLightCount; ++i)
		{
			mWorkingPointLights[i].Position.x = (i+1)*cosf(2*i + RJE::Math::Pi_f + timer );
			mWorkingPointLights[i].Position.y = 2.0f + cosf( timer );
			mWorkingPointLights[i].Position.z = (i+1)*sinf(2*i + RJE::Math::Pi_f + timer );
			light[i] = mWorkingPointLights[i];
		}
		mPointLights->Unmap(mDX11Device->md3dImmediateContext);
	}
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::DrawScene()
{
	RJE_ASSERT(mDX11Device->md3dImmediateContext);
	RJE_ASSERT(mSwapChain);

	PROFILE_GPU_START(L"Render Scene");
	PROFILE_GPU_START_DEEP(L"DEEP Scene");
	
	mDX11Device->md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, DirectX::Colors::LightSteelBlue);
	mDX11Device->md3dImmediateContext->ClearDepthStencilView(mDX11DepthBuffer->mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	//-------------------------------------------------------------------------

	DrawGizmos();

	//-------------------------------------------------------------------------
	mDX11Device->md3dImmediateContext->IASetInputLayout(DX11InputLayouts::PosNormalTex);
	mDX11Device->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sCurrentRasterizerState);

	float blendFactor[4] = {mBlendFactorR, mBlendFactorG, mBlendFactorB, mBlendFactorA};

	UINT stride = sizeof(MeshData::PosNormalTex);
	UINT offset = 0;
	mDX11Device->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mDX11Device->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	DirectX::XMMATRIX view     = mCamera->mView;
	DirectX::XMMATRIX proj     = *(mCamera->mCurrentProjectionMatrix);
	DirectX::XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	DX11Effects::BasicFX->SetDirLights(mDirLights);
	DX11Effects::BasicFX->SetPointLights(mPointLights->GetShaderResource());
	DX11Effects::BasicFX->SetEyePosW(mEyePosW);
	DX11Effects::BasicFX->SetSamplerState(DX11CommonStates::sCurrentSamplerState);
	DX11Effects::BasicFX->SetPointLightCount(mPointLightCount);
	DX11Effects::BasicFX->SetDirLightCount(mDirLightCount);
	DX11Effects::BasicFX->SetFogColor(      mScene.mFogColor);
	DX11Effects::BasicFX->SetFogStart(      mScene.mFogStart);
	DX11Effects::BasicFX->SetFogRange(      mScene.mFogRange);
	DX11Effects::BasicFX->SetFogState(      mScene.mbUseFog);
	DX11Effects::BasicFX->SetAlphaClipState(mScene.mbUseBlending);
	DX11Effects::BasicFX->SetTextureState(  mScene.mbUseTexture);

	ID3DX11EffectTechnique* activeTech = DX11Effects::BasicFX->BasicTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	Matrix44 world;
	Matrix44 worldInvTranspose;
	Matrix44 worldViewProj;

	activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the box.
		world             = mBoxWorld;
		worldInvTranspose = world;
		worldInvTranspose.InverseTranspose();
		worldViewProj     = world*view*proj;

		DX11Effects::BasicFX->SetWorld(world);
		DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
		DX11Effects::BasicFX->SetTexTransform(mBoxTexTransform);
		DX11Effects::BasicFX->SetMaterial(mBoxMat);

		if (mScene.mbUseBlending)
			mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullNone);
		mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sCurrentBlendState, blendFactor, 0xffffffff);
		activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
		mDX11Device->md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// restore the default rasterizer state
		mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sCurrentRasterizerState);
		mDX11Device->md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);

		// Draw the cylinders.
		for(int i = 0; i < 10; ++i)
		{
			world             = mCylWorld[i];
			worldInvTranspose = world;
			worldInvTranspose.InverseTranspose();
			worldViewProj     = world*view*proj;

			DX11Effects::BasicFX->SetWorld(world);
			DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
			DX11Effects::BasicFX->SetTexTransform(mCylinderTexTransform);
			DX11Effects::BasicFX->SetMaterial(mCylinderMat);

			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the model.
		world             = mModelWorld;
		worldInvTranspose = world;
		worldInvTranspose.InverseTranspose();
		worldViewProj     = world*view*proj;

		DX11Effects::BasicFX->SetWorld(world);
		DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
		DX11Effects::BasicFX->SetMaterial(mModelMat);

		activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
		mDX11Device->md3dImmediateContext->DrawIndexed(mModelIndexCount, mModelIndexOffset, mModelVertexOffset);

		// Draw the spheres.
		for(int i = 0; i < 10; ++i)
		{
			world             = mSphereWorld[i];
			worldInvTranspose = world;
			worldInvTranspose.InverseTranspose();
			worldViewProj     = world*view*proj;

			DX11Effects::BasicFX->SetWorld(world);
			DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
			DX11Effects::BasicFX->SetTexTransform(mSphereTexTransform);
			DX11Effects::BasicFX->SetMaterial(mSphereMat);

			if (mScene.mbUseBlending)
				mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullNone);
			mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sCurrentBlendState, blendFactor, 0xffffffff);
			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
			
			// Restore default render states
			mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sCurrentRasterizerState);
			mDX11Device->md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		}

		//////////////////////////////////////////////////////////////////////////
		
		// Draw the grid.
		world             = mGridWorld;
		worldInvTranspose = world;
		worldInvTranspose.InverseTranspose();
		worldViewProj     = world*view*proj;

		DX11Effects::BasicFX->SetWorld(world);
		DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
		DX11Effects::BasicFX->SetTexTransform(mGridTexTransform);
		DX11Effects::BasicFX->SetMaterial(mGridMat);

		// if we're in wireframe then we don't need to render the reflections
		if (mScene.mbWireframe || !mScene.mbDrawReflections )
		{
			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
		}
		else
		{
			// Do not write to render target.
			mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sBlendState_NoRenderTargetWrites, blendFactor, 0xffffffff);

			// Render visible mirror pixels to stencil buffer.
			// Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(DX11CommonStates::sDepthStencilState_MarkStencil, 1);

			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

			// Restore states.
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(0, 0);
			mDX11Device->md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);

			//////////////////////////////////////////////////////////////////////////

			// Draw the reflected meshes
			Vector4 mirrorPlane       = Vector4(0.0f, 1.0f, 0.0f, 0.0f);		// xz plane
			Matrix44 reflectionMatrix = Matrix44::Reflection(mirrorPlane);

			// Cache the old light directions, and reflect the light directions and position.
			Vector3 oldDirLightDirections[3];
			for(int i = 0; i < 3; ++i)
			{
				oldDirLightDirections[i]  = mDirLights[i].Direction;
				mDirLights[i].Direction  = Vector3::ReflectRay(mDirLights[i].Direction, Vector3::up);
			}
			if (mPointLightCount > 0)
			{
				PointLight* light = mPointLights->MapDiscard(mDX11Device->md3dImmediateContext);
				for (unsigned int i = 0; i < mPointLightCount; ++i)
				{
					mOldPointLights[i] = mWorkingPointLights[i];
					light[i] = mWorkingPointLights[i];
					light[i].Position = reflectionMatrix * mWorkingPointLights[i].Position;
				}
				mPointLights->Unmap(mDX11Device->md3dImmediateContext);
			}

			DX11Effects::BasicFX->SetPointLights(mPointLights->GetShaderResource());
			DX11Effects::BasicFX->SetDirLights(mDirLights);

			// Draw the box.
			world             = mBoxWorld * reflectionMatrix;
			worldInvTranspose = world;
			worldInvTranspose.InverseTranspose();
			worldViewProj     = world*view*proj;

			DX11Effects::BasicFX->SetWorld(world);
			DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
			DX11Effects::BasicFX->SetTexTransform(mBoxTexTransform);
			DX11Effects::BasicFX->SetMaterial(mBoxMat);

			// Cull clockwise triangles for reflection.
			if (mScene.mbUseBlending)
				mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullNone);
			else
				mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullClockwise);

			// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(DX11CommonStates::sDepthStencilState_DrawStenciled, 1);
			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

			mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullClockwise);

			// Draw the cylinders.
			for(int i = 0; i < 10; ++i)
			{
				world             = mCylWorld[i] * reflectionMatrix;
				worldInvTranspose = world;
				worldInvTranspose.InverseTranspose();
				worldViewProj     = world*view*proj;

				DX11Effects::BasicFX->SetWorld(world);
				DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
				DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
				DX11Effects::BasicFX->SetTexTransform(mCylinderTexTransform);
				DX11Effects::BasicFX->SetMaterial(mCylinderMat);

				// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
				mDX11Device->md3dImmediateContext->OMSetDepthStencilState(DX11CommonStates::sDepthStencilState_DrawStenciled, 1);
				activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
				mDX11Device->md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
			}

			if (mScene.mbUseBlending)
				mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullNone);
			else
				mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullClockwise);

			// Draw the spheres.
			for(int i = 0; i < 10; ++i)
			{
				world             = mSphereWorld[i] * reflectionMatrix;
				worldInvTranspose = world;
				worldInvTranspose.InverseTranspose();
				worldViewProj     = world*view*proj;

				DX11Effects::BasicFX->SetWorld(world);
				DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
				DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
				DX11Effects::BasicFX->SetTexTransform(mSphereTexTransform);
				DX11Effects::BasicFX->SetMaterial(mSphereMat);

				mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sCurrentBlendState, blendFactor, 0xffffffff);
				activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
				mDX11Device->md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

				// Restore default render states
				mDX11Device->md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
			}

			mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullClockwise);

			// Draw the model reflection
			world             = mModelWorld * reflectionMatrix;
			worldInvTranspose = world;
			worldInvTranspose.InverseTranspose();
			worldViewProj        = world*view*proj;

			DX11Effects::BasicFX->SetWorld(world);
			DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
			DX11Effects::BasicFX->SetMaterial(mModelMat);
			
			// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(DX11CommonStates::sDepthStencilState_DrawStenciled, 1);
			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mModelIndexCount, mModelIndexOffset, mModelVertexOffset);

			// Restore default states.
			mDX11Device->md3dImmediateContext->RSSetState(0);
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(0, 0);

			// Restore light settings.
			for(int i = 0; i < 3; ++i)
			{
				mDirLights[i].Direction  = oldDirLightDirections[i];
			}
			if (mPointLightCount > 0)
			{
				PointLight* light = mPointLights->MapDiscard(mDX11Device->md3dImmediateContext);
				for (unsigned int i = 0; i < mPointLightCount; ++i)
				{
					light[i] = mOldPointLights[i];
				}
				mPointLights->Unmap(mDX11Device->md3dImmediateContext);
			}
			DX11Effects::BasicFX->SetPointLights(mPointLights->GetShaderResource());
			DX11Effects::BasicFX->SetDirLights(mDirLights);

			// Draw the mirror to the back buffer as usual but with transparency
			// blending so the reflection shows through.
			world             = mGridWorld;
			worldInvTranspose = world;
			worldInvTranspose.InverseTranspose();
			worldViewProj     = world*view*proj;

			DX11Effects::BasicFX->SetWorld(world);
			DX11Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			DX11Effects::BasicFX->SetWorldViewProj(worldViewProj);
			DX11Effects::BasicFX->SetTexTransform(mGridTexTransform);
			DX11Effects::BasicFX->SetMaterial(mGridMat);

			mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sBlendState_Transparent, blendFactor, 0xffffffff);
			activeTech->GetPassByIndex(p)->Apply(0, mDX11Device->md3dImmediateContext);
			mDX11Device->md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

			// Restore default states.
			mDX11Device->md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
			mDX11Device->md3dImmediateContext->OMSetDepthStencilState(0, 0);
		}
	}
	
	mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_Solid);
	mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sBlendState_AlphaToCoverage, blendFactor, 0xffffffff);

	if (Console::Instance()->IsActive())
		DrawConsole();
	else if (Profiler::Instance()->IsActive())
		DrawProfiler();
	else if (!mScene.mbWireframe)
		Draw2dElements();

	//-------------------------------------------------------------------------

	PROFILE_GPU_END(L"Render Scene");
	PROFILE_GPU_END_DEEP(L"DEEP Scene");
	PROFILE_GPU_END_FRAME();

	//-------------------------------------------------------------------------

	// Draw the AntTweak GUI
	TwDraw();

	//-------------------------------------------------------------------------

	if (VSyncEnabled)
	{
		RJE_CHECK_FOR_SUCCESS(mSwapChain->Present(1, 0));
	}
	else
	{
		RJE_CHECK_FOR_SUCCESS(mSwapChain->Present(0, 0));
	}
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::DrawGizmos()
{
	PROFILE_CPU("Draw Gizmos");

	PROFILE_GPU_START(L"Render Gizmos");

	mDX11Device->md3dImmediateContext->IASetInputLayout(DX11InputLayouts::PosColor);
	mDX11Device->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//mDX11Device->md3dImmediateContext->RSSetState(DX11CommonStates::sRasterizerState_CullNone);

	UINT stride = sizeof(MeshData::ColorVertex);
	UINT offset = 0;
	mDX11Device->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer_Gizmo, &stride, &offset);
	mDX11Device->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer_Gizmo, DXGI_FORMAT_R32_UINT, 0);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	mDX11Device->md3dImmediateContext->OMSetBlendState(DX11CommonStates::sBlendState_Opaque, blendFactor, 0xffffffff);

	// Set constants
	Matrix44 view     = mCamera->mView;
	Matrix44 proj     = *(mCamera->mCurrentProjectionMatrix);
	Matrix44 viewProj = view*proj;

	Matrix44 world;
	Matrix44 worldViewProj;
	
	// Draw the wireBox.
	world         = mWireBoxWorld;
	worldViewProj = world*view*proj;

	DX11Effects::ColorFX->SetWorldViewProj(worldViewProj);

	DX11Effects::ColorFX->ColorTech->GetPassByIndex(0)->Apply(0, mDX11Device->md3dImmediateContext);
	mDX11Device->md3dImmediateContext->DrawIndexed(mWireBoxIndexCount, mWireBoxIndexOffset, mWireBoxVertexOffset);

	// Draw the Axis.
	world         = mAxisWorld;
	worldViewProj = world*view*proj;

	DX11Effects::ColorFX->SetWorldViewProj(worldViewProj);

	DX11Effects::ColorFX->ColorTech->GetPassByIndex(0)->Apply(0, mDX11Device->md3dImmediateContext);
	mDX11Device->md3dImmediateContext->DrawIndexed(mAxisIndexCount, mAxisIndexOffset, mAxisVertexOffset);

	PROFILE_GPU_END(L"Render Gizmos");
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::DrawConsole()
{
	PROFILE_CPU("Draw Console");

	char cmd[COMMAND_MAX_LENGTH];
	Console::Instance()->GetCommand(cmd);

	int consoleElevation = (int)Console::Instance()->mConsoleElevation;
	POINT textPos = {10, 5 + (LINE_MAX-Console::Instance()->GetLineCount()) * mConsoleFont->GetCharHeight() - consoleElevation};
	POINT cmdPos  = {10, CONSOLE_HEIGHT-25 - consoleElevation};

	CD3D11_RECT rect( 0, -consoleElevation, System::Instance()->mScreenWidth, CONSOLE_HEIGHT-consoleElevation);
	CD3D11_RECT rectLogo( System::Instance()->mScreenWidth - 220, 40-consoleElevation, System::Instance()->mScreenWidth - 30, 160-consoleElevation);
	
	mSpriteBatch->DrawInfoText(      *mConsoleFont, Console::Instance()->mConsoleBuffer, textPos);
	mSpriteBatch->DrawConsoleCommand(*mConsoleFont, cmd, cmdPos);

	mSpriteBatch->DrawTexture2D(mRjeLogo,           mDX11Device->md3dImmediateContext, rectLogo, 0xffffffff);
	mSpriteBatch->DrawTexture2D(mConsoleBackground, mDX11Device->md3dImmediateContext, rect,     0xffffffff);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::DrawProfiler()
{
	PROFILE_CPU("Draw Profiler");

	POINT profileStatsPos = {System::Instance()->mScreenWidth - 550, 20};
	POINT profileInfoPos  = {20, 70};

	std::wstring stats;
	stats =  L" CPU : " + AnsiToWString(System::Instance()->mCpuDescription) + L"\n";
	stats += L" GPU : " + wstring(System::Instance()->mGpuDescription)       + L"\n";
	stats += L" RAM : " + ToString(System::Instance()->mTotalSystemRAM)      + L" MB\n";
	stats += L"VRAM : " + ToString(System::Instance()->mGpuDedicatedVRAM)    + L" MB (Dedicated)\n";
	stats += L"VRAM : " + ToString(System::Instance()->mGpuSharedVRAM)       + L" MB (Shared)";
	mSpriteBatch->DrawString(*mProfilerFont, stats, profileStatsPos, XMCOLOR(0xffffff00));
	//---------------
	profileStatsPos.x = profileStatsPos.y = 20;
	stats =  L"FPS : "        + ToString(System::Instance()->fps);
	stats += L" (Min:"        + ToString(System::Instance()->minfps);
	stats += L" - Max:"       + ToString(System::Instance()->maxfps) + L")\n";
	stats += L"Time/Frame : " + ToString(System::Instance()->mspf) + L" ms";
	stats += L" (frame "      + ToString(System::Instance()->mTotalFrames) + L")";
	mSpriteBatch->DrawString(*mProfilerFont, stats, profileStatsPos, XMCOLOR(0xffffff00));
	//-----------------------------
	if (Profiler::Instance()->GetState() == PROFILER_STATES::E_GPU)
	{
#if RJE_PROFILE_GPU
		DX11Profiler::sInstance.GetProfilerInfo();
		mSpriteBatch->DrawInfoText(*mProfilerFont, DX11Profiler::sInstance.mProfileInfoString, profileInfoPos);
		profileInfoPos.x += 350;
		profileInfoPos.y = 125;
		mSpriteBatch->DrawInfoText(*mProfilerFont, DX11Profiler::sInstance.mProfileDeepInfoString, profileInfoPos);
#else
		mSpriteBatch->DrawInfoText(*mProfilerFont, "GPU Profiling Not Enabled", profileInfoPos);
#endif
	}
	else 
	{
#if RJE_PROFILE_CPU
		Profiler::Instance()->GetProfilerInfo();
		mSpriteBatch->DrawInfoText(*mProfilerFont, Profiler::Instance()->mProfileInfoString, profileInfoPos);
#else
		mSpriteBatch->DrawInfoText(*mProfilerFont, "CPU and Memory Profiling Not Enabled", profileInfoPos);
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::Draw2dElements()
{
	POINT textPos = {10, 10};

	std::wstring lightStat;
	lightStat =  L" Number of point lights : " + ToString(mPointLightCount);
	mSpriteBatch->DrawString(*mProfilerFont, lightStat, textPos, XMCOLOR(0xffffffff));
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::SetActivePointLights(UINT activeLights)
{
	mPointLightCount = activeLights;
	RJE_SAFE_DELETE(mPointLights);
	mPointLights = rje_new StructuredBuffer<PointLight>(mDX11Device->md3dDevice, activeLights, D3D11_BIND_SHADER_RESOURCE, true);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::Shutdown()
{
	//-----------
	// Terminate AntTweak GUI
	TwTerminate();
	//-----------
	
	PROFILE_GPU_EXIT();

	RJE_SAFE_RELEASE(mVertexBuffer);
	RJE_SAFE_RELEASE(mVertexBuffer_Gizmo);
	RJE_SAFE_RELEASE(mIndexBuffer);
	RJE_SAFE_RELEASE(mIndexBuffer_Gizmo);

	RJE_SAFE_RELEASE(mBoxMap);
	RJE_SAFE_RELEASE(mGridMap);
	RJE_SAFE_RELEASE(mSphereMap);
	RJE_SAFE_RELEASE(mCylinderMap);
	RJE_SAFE_RELEASE(mMaskMap);
	RJE_SAFE_RELEASE(mWhiteSRV);
	RJE_SAFE_RELEASE(mConsoleBackground);
	RJE_SAFE_RELEASE(mRjeLogo);

	DX11Effects     ::DestroyAll();
	DX11InputLayouts::DestroyAll();
	DX11CommonStates::DestroyAll();

	RJE_SAFE_DELETE(mPointLights);

	RJE_SAFE_DELETE(mSpriteBatch);
	RJE_SAFE_DELETE(mConsoleFont);
	RJE_SAFE_DELETE(mProfilerFont);

	RJE_SAFE_DELETE(mTextureMgr);

	RJE_SAFE_RELEASE(mRenderTargetView);
	mDX11DepthBuffer->Release();
	RJE_SAFE_RELEASE(mSwapChain);

	//------- Not used for now
// #ifdef RJE_DEBUG
// 	DXGI_DEBUG_RLO_FLAGS dbg_flags = DXGI_DEBUG_RLO_ALL;
// 	HRESULT result;
// 	// TODO: find why this is always returning E_INVALIDARG
// 	result = md3dDebug->ReportLiveObjects( DXGI_DEBUG_ALL, dbg_flags );
// #endif

	mDX11Device->Release();

	RJE_SAFE_DELETE(mDX11CommonStates);
	RJE_SAFE_DELETE(mDX11DepthBuffer);
	RJE_SAFE_DELETE(mDX11Device);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::ResizeWindow() {ResizeWindow(mWindowWidth, mWindowHeight);}
//-------------------------------------------
void DX11RenderingAPI::ResizeWindow(int newSizeWidth, int newSizeHeight)
{
	mWindowWidth  = newSizeWidth;
	mWindowHeight = newSizeHeight;

	RJE_ASSERT(mDX11Device->md3dImmediateContext);
	RJE_ASSERT(mDX11Device->md3dDevice);
	RJE_ASSERT(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	RJE_SAFE_RELEASE(mRenderTargetView);
	RJE_SAFE_RELEASE(mDX11DepthBuffer->mDepthStencilView);
	RJE_SAFE_RELEASE(mDX11DepthBuffer->mDepthStencilBuffer);

	// Resize the swap chain and recreate the render target view.
	RJE_CHECK_FOR_SUCCESS(mSwapChain->ResizeBuffers(1, newSizeWidth, newSizeHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH))
	ID3D11Texture2D* backBuffer;
	RJE_CHECK_FOR_SUCCESS(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
	RJE_SAFE_RELEASE(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width     = newSizeWidth;
	depthStencilDesc.Height    = newSizeHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDesc.SampleDesc.Count   = MSAA_Samples;
	depthStencilDesc.SampleDesc.Quality = 0;

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &(mDX11DepthBuffer->mDepthStencilBuffer)));
	RJE_CHECK_FOR_SUCCESS(mDX11Device->md3dDevice->CreateDepthStencilView(mDX11DepthBuffer->mDepthStencilBuffer, 0, &(mDX11DepthBuffer->mDepthStencilView)));


	// Bind the render target view and depth/stencil view to the pipeline.
	mDX11Device->md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDX11DepthBuffer->mDepthStencilView);

	// Set the viewport transform.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width    = static_cast<float>(newSizeWidth);
	mScreenViewport.Height   = static_cast<float>(newSizeHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mDX11Device->md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	mCamera->mSettings.AspectRatio = (float)newSizeWidth / (float)newSizeWidth;
	mCamera->UpdateProjMatrix((float)newSizeWidth, (float)newSizeHeight);
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::SetWireframe(BOOL state)
{
	if (state)
	{
		mScene.mbUseBlending = false;
		mScene.mbWireframe   = true;
		DX11CommonStates::sCurrentRasterizerState = DX11CommonStates::sRasterizerState_Wireframe;
		DX11CommonStates::sCurrentBlendState      = DX11CommonStates::sBlendState_Opaque;
	}
	else
	{
		mScene.mbWireframe = false;
		DX11CommonStates::sCurrentRasterizerState = DX11CommonStates::sRasterizerState_Solid;
	}
}

//////////////////////////////////////////////////////////////////////////
void DX11RenderingAPI::SetMSAA(UINT MSAASamples)
{
	InitSwapChain(MSAASamples);
	System::Instance()->OnResize();
}


//////////////////////////////////////////////////////////////////////////
// AntTweak CallBacks
//////////////////////////////////////////////////////////////////////////
void TW_CALL TwSetWireframe(void *clientData)
{
	DX11RenderingAPI* api = static_cast<DX11RenderingAPI*>(clientData);
	api->SetWireframe(!api->mScene.mbWireframe);
}