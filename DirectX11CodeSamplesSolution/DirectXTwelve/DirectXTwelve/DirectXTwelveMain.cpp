//--------------------------------------------------------------------------------------
// File: DrectXTwelveMain.cpp
//
// 
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "DXUTsettingsDlg.h"
#pragma warning( disable : 4100 )
#include "SimpleCamera.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

#pragma comment( lib, "comctl32.lib" )


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_STATIC_TEXT		   2
#define IDC_CHECKBOX_ONE		   3
#define IDC_SLIDER				   4

SimpleCamera             g_SimpleCamera;
CDXUTSDKMesh             g_Mesh;

XMMATRIX                            g_mWorld;
XMMATRIX                            g_mWView;
XMMATRIX                            g_mProjection;
ID3DX11EffectMatrixVariable*        g_pmWorld = nullptr;
ID3DX11EffectMatrixVariable*        g_pmWView = nullptr;
ID3DX11EffectMatrixVariable*        g_pmProjection = nullptr;

ID3DX11Effect*                      g_pEffectRole = nullptr;
ID3DX11EffectTechnique*             g_pTechRenderRole = nullptr;
ID3DX11EffectShaderResourceVariable*g_ptxDiffuseRole = nullptr;
ID3D11InputLayout*					g_pInputLayoutRole = nullptr;

ID3DX11EffectVectorVariable*        g_CameraPos = nullptr;
ID3DX11EffectVectorVariable*        g_Specular = nullptr;
ID3DX11EffectScalarVariable*        g_Power = nullptr;

XMVECTOR  g_LightPostion= XMVectorSet(0.0f, -200.0f, -100.0f,1.0f);

ID3DX11EffectVectorVariable*                   g_Ambient;
ID3DX11EffectVectorVariable*                   g_LightDir;

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 g_HUD;                  // dialog for standard controls
bool g_cheack_1 = false;


struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};
ID3D11Buffer * g_pVertexBuffer = nullptr;
ID3D11Buffer*  g_pIndexBuffer = nullptr;
ID3D11InputLayout * g_pInputLayoutCube = nullptr;
ID3D11ShaderResourceView*           g_pTextureCube = nullptr;
ID3DX11Effect*                      g_pEffectCube = nullptr;
ID3DX11EffectTechnique*             g_pTechRenderCube = nullptr;
ID3DX11EffectShaderResourceVariable*g_ptxDiffuseCube = nullptr;
ID3DX11EffectMatrixVariable*        g_pmWorldViewProjection = nullptr;

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
	V_RETURN(DXUTFindDXSDKMediaFileCch(szShaderPath, MAX_PATH, L"SpecularEffect.fx"));
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DX11CompileEffectFromFile(szShaderPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dwShaderFlags, 0, pd3dDevice, &g_pEffectRole, &pErrorBlob);
	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}
	if (FAILED(hr))
		return hr;

	
	V_RETURN(DXUTFindDXSDKMediaFileCch(szShaderPath, MAX_PATH, L"Tutorial07_effect.fx"));	
	hr = D3DX11CompileEffectFromFile(szShaderPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dwShaderFlags, 0, pd3dDevice, &g_pEffectCube, &pErrorBlob);
	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}
	if (FAILED(hr))
		return hr;

#endif

	// Obtain technique objects
	g_pTechRenderRole = g_pEffectRole->GetTechniqueByName("RenderCubeWithTexture");
	ID3DX11EffectVariable* effect11_variable = g_pEffectRole->GetVariableByName("g_MeshTexture");
	g_ptxDiffuseRole = effect11_variable->AsShaderResource();

	effect11_variable = g_pEffectRole->GetVariableByName("g_mWorld");
	g_pmWorld = effect11_variable->AsMatrix();
	effect11_variable = g_pEffectRole->GetVariableByName("g_mView");
	g_pmWView = effect11_variable->AsMatrix();
	effect11_variable = g_pEffectRole->GetVariableByName("g_mProjection");
	g_pmProjection = effect11_variable->AsMatrix();

	effect11_variable = g_pEffectRole->GetVariableByName("g_Ambient");
	g_Ambient = effect11_variable->AsVector();
	effect11_variable = g_pEffectRole->GetVariableByName("g_LightDir");
	g_LightDir = effect11_variable->AsVector();

	effect11_variable = g_pEffectRole->GetVariableByName("g_CameraPos");
	g_CameraPos = effect11_variable->AsVector();
	effect11_variable = g_pEffectRole->GetVariableByName("g_Specular");
	g_Specular = effect11_variable->AsVector();
	effect11_variable = g_pEffectRole->GetVariableByName("g_Power");
	g_Power = effect11_variable->AsScalar();

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout_role[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout_role);

	//// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	ID3DX11EffectPass* dx11_effect_pass;
	dx11_effect_pass = g_pTechRenderRole->GetPassByIndex(0);
	V_RETURN(dx11_effect_pass->GetDesc(&PassDesc));
	V_RETURN(pd3dDevice->CreateInputLayout(layout_role, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pInputLayoutRole));

	ID3D11DeviceContext * g_pImmediateContext = DXUTGetD3D11DeviceContext();
	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pInputLayoutRole);

	//Load the mesh
	V_RETURN(g_Mesh.Create(pd3dDevice, L"Tiny\\tiny.sdkmesh"));//"Tiny\\tiny.sdkmesh"

	//---------------------------------------------------------------------------------------------

	g_pTechRenderCube = g_pEffectCube->GetTechniqueByName("RenderCubeWithTexture");

	effect11_variable = g_pEffectCube->GetVariableByName("g_MeshTexture");
	g_ptxDiffuseCube = effect11_variable->AsShaderResource();

	effect11_variable = g_pEffectCube->GetVariableByName("g_mWorldViewProjection");
	g_pmWorldViewProjection = effect11_variable->AsMatrix();

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = ARRAYSIZE(layout);

	//// Create the input layout
	//D3DX11_PASS_DESC PassDesc;
	//ID3DX11EffectPass* dx11_effect_pass;
	dx11_effect_pass = g_pTechRenderCube->GetPassByIndex(0);
	V_RETURN(dx11_effect_pass->GetDesc(&PassDesc));
	V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pInputLayoutCube));

	//ID3D11DeviceContext * g_pImmediateContext = DXUTGetD3D11DeviceContext();
	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pInputLayoutCube);

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
	hr = CreateDDSTextureFromFile(pd3dDevice, L"ship.dds", nullptr, &g_pTextureCube);//seafloor.dds
	if (FAILED(hr))
		return hr;

	OutputDebugString(L"准备--12\n");

	// Initialize the world matrices
	g_mWorld = XMMatrixIdentity();

	// update the 3D projection matrix	
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_SimpleCamera.SetProjectionParameters(XM_PIDIV2, fAspectRatio, 0.01f, 5000.0f);
	g_SimpleCamera.GetProjectionMatrix(&g_mProjection);
    g_SimpleCamera.SetViewParameters(XMFLOAT3(0.0f, 450.0f, -300.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	//g_SimpleCamera.SetViewParameters(XMFLOAT3(1.3f, 1.1f, -3.3f), XMFLOAT3(0.75f, 0.9f, -2.5f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	g_SimpleCamera.GetViewMatrix(&g_mWView);

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
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_SimpleCamera.SetProjectionParameters(XM_PIDIV2, fAspectRatio, 0.01f, 5000.0f);
	g_SimpleCamera.GetProjectionMatrix(&g_mProjection);
	g_SimpleCamera.SetViewParameters(XMFLOAT3(0.0f, 450.0f, -300.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	g_SimpleCamera.GetViewMatrix(&g_mWView);


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

	XMFLOAT4X4 m;

	// Initialize the world matrices
	g_mWorld = XMMatrixRotationY(XMConvertToRadians(180.f));//180.f
	XMMATRIX mRot = XMMatrixRotationX(XMConvertToRadians(-90.0f));
	g_mWorld = mRot * g_mWorld;

	if(g_cheack_1)
	{
		// Update our time
		static float t = 0.0f;

		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;

		// Rotate cube around the origin
		g_mWorld *= XMMatrixRotationY(t);
	}
	

	//XMMATRIX mWorldViewProjection = g_mWorld * g_mWView * g_mProjection;
	g_mWorld *= XMMatrixTranslation(0, 0, -150);

	XMVECTOR  rolepos = g_mWorld.r[3];
	XMVECTOR  light_dir = g_LightPostion - rolepos;
	light_dir = XMVector3Normalize(light_dir);
	g_LightDir->SetFloatVector(light_dir.m128_f32);
	
	XMStoreFloat4x4(&m, g_mWorld);
	g_pmWorld->SetMatrix((float*)&m);

	XMStoreFloat4x4(&m, g_mWView);
	g_pmWView->SetMatrix((float*)&m);

	XMStoreFloat4x4(&m, g_mProjection);
	g_pmProjection->SetMatrix((float*)&m);

	float coler[4] = { 0.1f,0.1f,0.1f,1.0f };
	//float coler[4] = { 1.0f,0.0f,0.0f,1.0f };
	g_Ambient->SetFloatVector(coler);

	//float lightdir[4] = { -0.577f, 0.577f, -0.577f, 1.0f };
	//float f[4]= light_dir.m128_f32;
	//g_LightDir->SetFloatVector(light_dir.m128_f32);

	float cameravector[4] = { 0.0f, 200.0f, -400.0f ,1.0f};
	g_CameraPos->SetFloatVector(cameravector);

	float specular[4] = { 1.0f,1.0f,1.0f,1.0f };
	//float coler[4] = { 1.0f,0.0f,0.0f,1.0f };
	g_Specular->SetFloatVector(specular);

	float power = 16.0f;
	g_Power->SetFloat(power);

	//
	// Set the Vertex Layout
	//
	pd3dImmediateContext->IASetInputLayout(g_pInputLayoutRole);

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
		g_ptxDiffuseRole->SetResource(pDiffuseRV);

		D3DX11_TECHNIQUE_DESC techDesc;
		V(g_pTechRenderRole->GetDesc(&techDesc));

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTechRenderRole->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
			pd3dImmediateContext->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
		}
	}

	g_mWorld *= XMMatrixTranslation(300, 0, 500);
	XMStoreFloat4x4(&m, g_mWorld);
	g_pmWorld->SetMatrix((float*)&m);

	  rolepos = g_mWorld.r[3];
	  light_dir = g_LightPostion - rolepos;
	light_dir = XMVector3Normalize(light_dir);
	g_LightDir->SetFloatVector(light_dir.m128_f32);


	for (UINT subset = 0; subset < g_Mesh.GetNumSubsets(0); ++subset)
	{
		auto pSubset = g_Mesh.GetSubset(0, subset);

		auto PrimType = g_Mesh.GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		pd3dImmediateContext->IASetPrimitiveTopology(PrimType);

		// Ignores most of the material information in them mesh to use only a simple shader
		auto pDiffuseRV = g_Mesh.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		g_ptxDiffuseRole->SetResource(pDiffuseRV);

		D3DX11_TECHNIQUE_DESC techDesc;
		V(g_pTechRenderRole->GetDesc(&techDesc));

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTechRenderRole->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
			pd3dImmediateContext->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
		}
	}

	g_mWorld *= XMMatrixTranslation(-600, 0, 0);
	XMStoreFloat4x4(&m, g_mWorld);
	g_pmWorld->SetMatrix((float*)&m);


	rolepos = g_mWorld.r[3];
	light_dir = g_LightPostion - rolepos;
	light_dir = XMVector3Normalize(light_dir);
	g_LightDir->SetFloatVector(light_dir.m128_f32);


	for (UINT subset = 0; subset < g_Mesh.GetNumSubsets(0); ++subset)
	{
		auto pSubset = g_Mesh.GetSubset(0, subset);

		auto PrimType = g_Mesh.GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		pd3dImmediateContext->IASetPrimitiveTopology(PrimType);

		// Ignores most of the material information in them mesh to use only a simple shader
		auto pDiffuseRV = g_Mesh.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		g_ptxDiffuseRole->SetResource(pDiffuseRV);

		D3DX11_TECHNIQUE_DESC techDesc;
		V(g_pTechRenderRole->GetDesc(&techDesc));

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTechRenderRole->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
			pd3dImmediateContext->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
		}
	}


	pd3dImmediateContext->IASetInputLayout(g_pInputLayoutCube);
	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	// Set index buffer
	pd3dImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	float light_pos_x= XMVectorGetX(g_LightPostion);    //得到XMVECTOR的X维数据  
	float light_pos_y = XMVectorGetY(g_LightPostion);   //得到XMVECTOR的Y维数据  
	float light_pos_z = XMVectorGetZ(g_LightPostion);    //得到XMVECTOR的Z维数据 


	g_mWorld = XMMatrixIdentity();
	g_mWorld*=XMMatrixScaling(10.0f, 10.0f, 10.0f)*XMMatrixTranslation(light_pos_x, light_pos_y, light_pos_z);

	XMMATRIX mWorldViewProjection = g_mWorld * g_mWView * g_mProjection;
//	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, mWorldViewProjection);
	g_pmWorldViewProjection->SetMatrix((float*)&m);
	g_ptxDiffuseCube->SetResource(g_pTextureCube);
	//
	// Render the cube
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	V(g_pTechRenderCube->GetDesc(&techDesc));

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		g_pTechRenderCube->GetPassByIndex(p)->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->DrawIndexed(36, 0, 0);
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
	SAFE_RELEASE(g_pInputLayoutRole);
	SAFE_RELEASE(g_pEffectRole);
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pInputLayoutCube);
	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pTextureCube);
	SAFE_RELEASE(g_pEffectCube);

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
	case IDC_CHECKBOX_ONE:
		g_cheack_1 = ((CDXUTCheckBox*)pControl)->GetChecked();		
		break;
	case IDC_SLIDER:
		//swprintf_s(sz, 100, L"Slider Value: %d", g_HUD.GetSlider(IDC_SLIDER)->GetValue());
		k = g_HUD.GetSlider(IDC_SLIDER)->GetValue();
		//XMVECTOR  g_LightPostion = XMVectorSet(0.0f, -200.0f, -100.0f, 1.0f);
		g_LightPostion= XMVectorSetX(g_LightPostion, k);
	//	g_HUD.GetStatic(IDC_STATIC_TEXT)->SetText(sz);
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
	g_HUD.AddCheckBox(IDC_CHECKBOX_ONE, L"Rotate",35, iY += 24, 150, 24, false, L'R');
	g_HUD.AddSlider(IDC_SLIDER, 0, iY += 24, 150, 30, -500, 500, 0);
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
	DXUTCreateWindow(L"DrectXTwelve");

	// Only require 10-level hardware or later
	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

					// Perform any application-level cleanup here
	return DXUTGetExitCode();
}


