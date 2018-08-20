//--------------------------------------------------------------------------------------
// File: DirectXTenMain.cpp
//
// 学习和使用摄像机
//
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "DXUTsettingsDlg.h"
#pragma warning( disable : 4100 )
#include "SimpleCamera.h"

using namespace DirectX;

#pragma comment( lib, "comctl32.lib" )


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_STATIC_TEXT		   2


SimpleCamera             g_SimpleCamera;
CDXUTSDKMesh             g_Mesh;

XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

ID3DX11Effect*                      g_pEffect = nullptr;
ID3DX11EffectTechnique*             g_pTechRenderCubeWithTexture = nullptr;
ID3DX11EffectShaderResourceVariable*g_ptxDiffuse = nullptr;
ID3DX11EffectMatrixVariable*        g_pmWorldViewProjection = nullptr;
ID3DX11EffectMatrixVariable*        g_pmWorld = nullptr;

ID3D11InputLayout * g_pInputLayout = nullptr;
ID3D11ShaderResourceView*           g_pTextureRV = nullptr;

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 g_HUD;                  // dialog for standard controls

ID3DX11EffectVectorVariable*                   g_Ambient;
ID3DX11EffectVectorVariable*                   g_LightDir;


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr = S_OK;

	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));



	// Compile and create the effect.
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

#if D3D_COMPILER_VERSION >= 46

	WCHAR szShaderPath[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(szShaderPath, MAX_PATH, L"DiffuseEffect.fx"));
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DX11CompileEffectFromFile(szShaderPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dwShaderFlags, 0, pd3dDevice, &g_pEffect, &pErrorBlob);
	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}
	if (FAILED(hr))
		return hr;
	//#else
	//	ID3DBlob* pEffectBuffer = nullptr;
	//	V_RETURN(DXUTCompileFromFile(L"Tutorial07_effect.fx", nullptr, "none", "fx_5_0", dwShaderFlags, 0, &pEffectBuffer));
	//	hr = D3DX11CreateEffectFromMemory(pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), 0, pd3dDevice, &g_pEffect);
	//	SAFE_RELEASE(pEffectBuffer);
	//	if (FAILED(hr))
	//		return hr;
#endif

	// Obtain technique objects
	g_pTechRenderCubeWithTexture = g_pEffect->GetTechniqueByName("RenderCubeWithTexture");
	ID3DX11EffectVariable* effect11_variable = g_pEffect->GetVariableByName("g_MeshTexture");
	g_ptxDiffuse = effect11_variable->AsShaderResource();

	effect11_variable = g_pEffect->GetVariableByName("g_mWorldViewProjection");
	g_pmWorldViewProjection = effect11_variable->AsMatrix();
	effect11_variable = g_pEffect->GetVariableByName("g_mWorld");
	g_pmWorld = effect11_variable->AsMatrix();

	effect11_variable = g_pEffect->GetVariableByName("g_Ambient");
	g_Ambient = effect11_variable->AsVector();
	effect11_variable = g_pEffect->GetVariableByName("g_LightDir");
	g_LightDir = effect11_variable->AsVector();




	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	//// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	ID3DX11EffectPass* dx11_effect_pass;
	dx11_effect_pass = g_pTechRenderCubeWithTexture->GetPassByIndex(0);
	V_RETURN(dx11_effect_pass->GetDesc(&PassDesc));
	V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pInputLayout));

	ID3D11DeviceContext * g_pImmediateContext = DXUTGetD3D11DeviceContext();
	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	//Load the mesh
	V_RETURN(g_Mesh.Create(pd3dDevice, L"Tiny\\tiny.sdkmesh"));//"Tiny\\tiny.sdkmesh"

															   // Initialize the world matrices
	g_World = XMMatrixIdentity();

	// update the 3D projection matrix	
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_SimpleCamera.SetProjectionParameters(XM_PIDIV2, fAspectRatio, 0.01f, 1000.0f);
	g_SimpleCamera.GetProjectionMatrix(&g_Projection);
	g_SimpleCamera.SetViewParameters(XMFLOAT3(0.0f, 250.0f, -300.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	g_SimpleCamera.GetViewMatrix(&g_View);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{

	HRESULT hr;
	V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_HUD.SetSize(170, 170);

	// Setup the camera's projection parameters

	//g_SimpleCamera.SetOrthographicParameters(60, 50, -100, 100);
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_SimpleCamera.SetProjectionParameters(XM_PIDIV2, fAspectRatio, 0.01f, 1000.0f);
	g_SimpleCamera.GetProjectionMatrix(&g_Projection);
	g_SimpleCamera.SetViewParameters(XMFLOAT3(0.0f, 250.0f, -300.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	g_SimpleCamera.GetViewMatrix(&g_View);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{

}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	HRESULT hr;

	// Clear render target and the depth stencil 
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::MidnightBlue);

	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	// Initialize the world matrices
	g_World = XMMatrixRotationY(XMConvertToRadians(180.f));
	XMMATRIX mRot = XMMatrixRotationX(XMConvertToRadians(-90.0f));
	g_World = mRot * g_World;
	XMMATRIX mWorldViewProjection = g_World * g_View * g_Projection;

	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, mWorldViewProjection);
	g_pmWorldViewProjection->SetMatrix((float*)&m);


	XMStoreFloat4x4(&m, g_World);
	g_pmWorld->SetMatrix((float*)&m);

	float coler[4] = { 0.1f,0.1f,0.1f,0.3f };
	//float coler[4] = { 1.0f,0.0f,0.0f,1.0f };
	g_Ambient->SetFloatVector(coler);

	float lightdir[4] = { -0.577f, 0.577f, -0.577f, 0.f };
	g_LightDir->SetFloatVector(lightdir);
	//
	// Set the Vertex Layout
	//
	pd3dImmediateContext->IASetInputLayout(g_pInputLayout);

	//
	// Render the mesh
	//	
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = g_Mesh.GetVB11(0, 0);
	Strides[0] = (UINT)g_Mesh.GetVertexStride(0, 0);
	Offsets[0] = 0;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, pVB, Strides, Offsets);
	pd3dImmediateContext->IASetIndexBuffer(g_Mesh.GetIB11(0), g_Mesh.GetIBFormat11(0), 0);


	for (UINT subset = 0; subset < g_Mesh.GetNumSubsets(0); ++subset)
	{
		auto pSubset = g_Mesh.GetSubset(0, subset);

		auto PrimType = g_Mesh.GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		pd3dImmediateContext->IASetPrimitiveTopology(PrimType);

		// Ignores most of the material information in them mesh to use only a simple shader
		auto pDiffuseRV = g_Mesh.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		g_ptxDiffuse->SetResource(pDiffuseRV);

		D3DX11_TECHNIQUE_DESC techDesc;
		V(g_pTechRenderCubeWithTexture->GetDesc(&techDesc));

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTechRenderCubeWithTexture->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
			pd3dImmediateContext->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
		}
	}

	g_HUD.OnRender(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{

	g_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	g_Mesh.Destroy();
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pTextureRV);
	SAFE_RELEASE(g_pEffect);

}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	WCHAR sz[100];
	bool res = false;
	int k = 0;
	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;

	}
}
//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	// Perform any application-level initialization here
	g_HUD.Init(&g_DialogResourceManager);
	g_HUD.SetCallback(OnGUIEvent);
	int iY = 10;
	g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 23);//
	g_HUD.AddStatic(IDC_STATIC_TEXT, L"Quit: ESC", 35, iY += 24, 300, 22);
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// DXUT will create and use the best device
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackMouse(OnMouse);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackDeviceRemoved(OnDeviceRemoved);

	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	// Perform any application-level initialization here
	DXUTInit(true, true, nullptr); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen

	InitApp();
	DXUTCreateWindow(L"DrectXTen");

	// Only require 10-level hardware or later
	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

					// Perform any application-level cleanup here
	return DXUTGetExitCode();
}


