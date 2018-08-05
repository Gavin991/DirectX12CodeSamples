//--------------------------------------------------------------------------------------
// File: DirectXFiveMain.cpp
//
// 渲染一个带纹理贴图的立方体
//
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DDSTextureLoader.h"
#include "d3dx11effect.h"
#include "SDKMisc.h"

#pragma warning( disable : 4100 )

using namespace DirectX;

#pragma comment( lib, "comctl32.lib" )


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};


ID3D11Buffer * g_pVertexBuffer = nullptr;
ID3D11Buffer*  g_pIndexBuffer = nullptr;

ID3D11InputLayout * g_pInputLayout = nullptr;

ID3D11ShaderResourceView*           g_pTextureRV = nullptr;

XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

ID3DX11Effect*                      g_pEffect = nullptr;
ID3DX11EffectTechnique*             g_pTechRenderCubeWithTexture = nullptr;


ID3DX11EffectShaderResourceVariable*g_ptxDiffuse = nullptr;
ID3DX11EffectMatrixVariable*        g_pmWorldViewProjection = nullptr;


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
// 此处要加载d3dcompiler.lib库
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

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
	//此处要加载d3dcompiler.lib库
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}


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
	V_RETURN(DXUTFindDXSDKMediaFileCch(szShaderPath, MAX_PATH, L"Tutorial07_effect.fx"));
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
	// Obtain variables
	//g_ptxDiffuse = g_pEffect->GetVariableByName("g_MeshTexture")->AsShaderResource();
	//g_pmWorldViewProjection = g_pEffect->GetVariableByName("g_mWorldViewProjection")->AsMatrix();

	ID3DX11EffectVariable* effect11_variable = g_pEffect->GetVariableByName("g_MeshTexture");
	g_ptxDiffuse = effect11_variable->AsShaderResource();

	effect11_variable = g_pEffect->GetVariableByName("g_mWorldViewProjection");
	g_pmWorldViewProjection = effect11_variable->AsMatrix();

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	OutputDebugString(L"准备\n");
	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, -2.0f, 2.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.0f, -2.0f, 2.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-2.0f, -2.0f, -2.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.0f, -2.0f, -2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, -2.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-2.0f, -2.0f, 2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, -2.0f, 2.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.0f, 2.0f, 2.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	OutputDebugString(L"准备--8\n");
	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	OutputDebugString(L"准备--9\n");
	// Create index buffer
	// Create vertex buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Load the Texture
	hr = CreateDDSTextureFromFile(pd3dDevice, L"ship.dds", nullptr, &g_pTextureRV);//seafloor.dds
	if (FAILED(hr))
		return hr;

	OutputDebugString(L"准备--12\n");

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800 / (FLOAT)600, 0.01f, 100.0f);


	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
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



	// Update our time
	static float t = 0.0f;

	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	t = (timeCur - timeStart) / 1000.0f;

	// Rotate cube around the origin
	g_World = XMMatrixRotationY(t);

	XMMATRIX mWorldViewProjection = g_World * g_View * g_Projection;
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, mWorldViewProjection);
	g_pmWorldViewProjection->SetMatrix((float*)&m);
	g_ptxDiffuse->SetResource(g_pTextureRV);
	//
	// Render the cube
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	V(g_pTechRenderCubeWithTexture->GetDesc(&techDesc));

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pTechRenderCubeWithTexture->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->DrawIndexed(36, 0, 0);
	}


}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pInputLayout);

	SAFE_RELEASE(g_pIndexBuffer);

	SAFE_RELEASE(g_pTextureRV);


	SAFE_RELEASE(g_pEffect);


}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext)
{
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
	DXUTCreateWindow(L"DrectXFive");

	// Only require 10-level hardware or later
	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

					// Perform any application-level cleanup here

	return DXUTGetExitCode();
}


