//--------------------------------------------------------------------------------------
// File: DirectXOneMain.cpp
//
// Empty starting point for new Direct3D 11 Win32 desktop applications
//
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "DXUTguiIME.h"

#pragma warning( disable : 4100 )

using namespace DirectX;

#pragma comment( lib, "comctl32.lib" )
//#pragma comment(lib, "version.lib")
//#include "Imm.h"
//#pragma comment(lib,"imm32.lib")

void InitApp();
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_STATIC_ESC		   2

#define IDC_STATIC_TEXT		   3
#define IDC_SLIDER				   4

#define IDC_STATIC_RADIO_VALUE		       5
#define IDC_STATIC_CHECKBOX_VALUE           6
#define IDC_STATIC_COMBOBOX_VALUE           7

#define IDC_COMBOBOX            9

#define IDC_CHECKBOX_ONE            14
#define IDC_CHECKBOX_TWO           15

#define IDC_RADIO1A             16
#define IDC_RADIO1B             17
#define IDC_RADIO1C             18
#define IDC_RADIO2A             19
#define IDC_RADIO2B             20
#define IDC_RADIO2C             21

#define IDC_LISTBOX             22
#define IDC_LISTBOXM            23

#define IDC_EDITBOX1 99
#define IDC_EDITBOX2 100

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 g_HUD;                   // manages the 3D   
CDXUTDialog                 g_SampleUI;              // dialog for sample specific controls
CDXUTDialog                 g_LeftPlaneUI;           


int g_radio_1 = 0;
int g_radio_2 = 0;
bool g_cheack_1 = false;
bool g_cheack_2 = false;

void RadiValueShow(int radioValue1, int radiowValua2);
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
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	WCHAR sz[100];
	bool res=false;
	int k=0;
	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;

	case IDC_SLIDER:		
		swprintf_s(sz, 100, L"Slider Value: %d", g_SampleUI.GetSlider(IDC_SLIDER)->GetValue());
		g_SampleUI.GetStatic(IDC_STATIC_TEXT)->SetText(sz);
		break;
	case IDC_RADIO1A:
		g_radio_1 = 1;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_RADIO1B:
		g_radio_1 = 2;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_RADIO1C:
		g_radio_1 = 3;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_RADIO2A:
		g_radio_2 = 1;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_RADIO2B:
		g_radio_2 = 2;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_RADIO2C:
		g_radio_2 = 3;
		RadiValueShow(g_radio_1, g_radio_2);
		break;
	case IDC_CHECKBOX_ONE:
		g_cheack_1 = ((CDXUTCheckBox*)pControl )->GetChecked();
		swprintf_s(sz, 100, L"CheckBox1 Value: %d ; CheckBox2: %d ;", g_cheack_1, g_cheack_2);
		g_SampleUI.GetStatic(IDC_STATIC_CHECKBOX_VALUE)->SetText(sz);
		break;
	case IDC_CHECKBOX_TWO:
		g_cheack_2 = ((CDXUTCheckBox*)pControl)->GetChecked();
		swprintf_s(sz, 100, L"CheckBox1 Value: %d ; CheckBox2: %d ;", g_cheack_1, g_cheack_2);
		g_SampleUI.GetStatic(IDC_STATIC_CHECKBOX_VALUE)->SetText(sz);
		break;
	case IDC_COMBOBOX:
		void *p = ((CDXUTComboBox*)pControl)->GetSelectedData();
		k= (int)p;	
		swprintf_s(sz, 100, L"ComboBox Value: %d ; ", k);
		g_SampleUI.GetStatic(IDC_STATIC_COMBOBOX_VALUE)->SetText(sz);
		break;

	}

}

void RadiValueShow(int radioValue1,int radiowValua2)
{
	WCHAR sz[100];
	swprintf_s(sz, 100, L"RADIO1 Value: %d ; RADIO2 Value: %d ;", g_radio_1, g_radio_2);
	g_SampleUI.GetStatic(IDC_STATIC_RADIO_VALUE)->SetText(sz);
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;
	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));

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

	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 300, pBackBufferSurfaceDesc->Height - 550);
	g_SampleUI.SetSize(300, 300);

	g_LeftPlaneUI.SetLocation(0,0);
	g_LeftPlaneUI.SetSize(300, 600);
	
	OutputDebugString(L"准备--9\n");
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
	// Clear render target and the depth stencil 
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::MidnightBlue);

	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	g_HUD.OnRender(fElapsedTime);
	g_SampleUI.OnRender(fElapsedTime);
	g_LeftPlaneUI.OnRender(fElapsedTime);
	DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	//g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11DestroyDevice();

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
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = g_LeftPlaneUI.MsgProc(hWnd, uMsg, wParam, lParam);
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


	InitApp();
	DXUTInit(true, true, nullptr); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
	DXUTCreateWindow(L"DrectXSix");

	// Only require 10-level hardware or later
	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

					// Perform any application-level cleanup here

	return DXUTGetExitCode();
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

	g_SampleUI.Init(&g_DialogResourceManager);
	g_SampleUI.SetCallback(OnGUIEvent); 
	iY = 10;

	WCHAR sz[100];
	iY += 24;
	swprintf_s(sz, 100, L"Static Text: %s", L"This is Static Text");
	g_SampleUI.AddStatic(IDC_STATIC_TEXT, sz, 35, iY += 24,  300, 22);
	g_SampleUI.GetStatic(IDC_STATIC_TEXT)->SetTextColor(D3DCOLOR_ARGB(255, 0, 255, 0)); // 绿色字体
	g_SampleUI.AddSlider(IDC_SLIDER, 35, iY += 24, 200,  30, 1, 100, 50);

	iY += 24;
	swprintf_s(sz, 100, L"RADIO1 Value: %s ; RADIO2 Value: %s ;", L" ",  L" ");
	g_SampleUI.AddStatic(IDC_STATIC_RADIO_VALUE, sz, 35, iY +=24, 300, 22);
	g_SampleUI.GetStatic(IDC_STATIC_RADIO_VALUE)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 0)); // 	
	// Radio buttons
	g_SampleUI.AddRadioButton(IDC_RADIO1A, 1, L"Radio group 1 Amy (1)", 35, iY += 24, 220, 24, false, L'1');
	g_SampleUI.AddRadioButton(IDC_RADIO1B, 1, L"Radio group 1 Brian (2)", 35, iY += 24, 220, 24, false, L'2');
	g_SampleUI.AddRadioButton(IDC_RADIO1C, 1, L"Radio group 1 Clark (3)", 35, iY += 24, 220, 24, false, L'3');
	iY += 10;
	g_SampleUI.AddRadioButton(IDC_RADIO2A, 2, L"Single (4)", 35, iY += 24, 90, 24, false, L'4');
	g_SampleUI.AddRadioButton(IDC_RADIO2B, 2, L"Double (5)", 35, iY += 24, 90, 24, false, L'5');
	g_SampleUI.AddRadioButton(IDC_RADIO2C, 2, L"Triple (6)", 35, iY += 24, 90, 24, false, L'6');
	iY += 10;

	swprintf_s(sz, 100, L"CheckBox1 Value: %s ; CheckBox2: %s ;", L" ", L" ");	
	g_SampleUI.AddStatic(IDC_STATIC_CHECKBOX_VALUE, sz, 35, iY += 24, 300, 22);
	g_SampleUI.GetStatic(IDC_STATIC_CHECKBOX_VALUE)->SetTextColor(D3DCOLOR_ARGB(255, 255, 165, 0)); // 
	// Checkbox
	g_SampleUI.AddCheckBox(IDC_CHECKBOX_ONE, L"Checkbox One (C)",
		35, iY += 24, 150, 24, false, L'C');//, false
	g_SampleUI.AddCheckBox(IDC_CHECKBOX_TWO,	L"Checkbox Two (T)",
		35, iY += 24, 150, 24, false, L'T');//, false

	swprintf_s(sz, 100, L"ComboBox Value: %s ; ", L" ");
	g_SampleUI.AddStatic(IDC_STATIC_COMBOBOX_VALUE, sz, 35, iY += 24, 300, 22);
	g_SampleUI.GetStatic(IDC_STATIC_COMBOBOX_VALUE)->SetTextColor(D3DCOLOR_ARGB(255, 0, 255, 255)); //

	iY += 10;
	CDXUTComboBox* pCombo;
	g_SampleUI.AddComboBox(IDC_COMBOBOX, 35, iY += 24,  250, 24, L'O', false, &pCombo);
	if (pCombo)
	{
		pCombo->SetDropHeight(100);
		pCombo->AddItem(L"Combobox item (O)", (void*) 0 );//(LPVOID)0x11111111
		pCombo->AddItem(L"Placeholder (O)", (void*) 1 );
		pCombo->AddItem(L"One more (O)", (void*) 2 );
		pCombo->AddItem(L"I can't get enough (O)", (void*) 3 );
		pCombo->AddItem(L"Ok, last one, I promise (O)", (void*) 4 );
	}
	
	g_LeftPlaneUI.Init(&g_DialogResourceManager);
	g_LeftPlaneUI.SetCallback(OnGUIEvent);
	iY = 20;

	//g_LeftPlaneUI.AddListBox(IDC_LISTBOX, 30, iY += 24, 250, 150, 0);
	//for (int i = 0; i < 15; ++i)
	//{
	//	WCHAR wszText[50];
	//	swprintf_s(wszText, 50, L"Single-selection listbox item %d", i);
	//	g_LeftPlaneUI.GetListBox(IDC_LISTBOX)->AddItem(wszText, (LPVOID)(size_t)i);
	//}

	//g_LeftPlaneUI.AddListBox(IDC_LISTBOXM, 30, iY += 200, 250, 150, CDXUTListBox::MULTISELECTION);
	//for (int i = 0; i < 30; ++i)
	//{
	//	WCHAR wszText[50];
	//	swprintf_s(wszText, 50, L"Multi-selection listbox item %d", i);
	//	g_LeftPlaneUI.GetListBox(IDC_LISTBOXM)->AddItem(wszText, (LPVOID)(size_t)i);
	//}

	// Edit box
	//g_LeftPlaneUI.AddEditBox(IDC_EDITBOX1, L"Edit control with default styles. Type text here and press Enter", 20, iY += 250,	600, 32);


	//// IME-enabled edit box
	//CDXUTIMEEditBox* pIMEEdit;
	//CDXUTIMEEditBox::InitDefaultElements(&g_LeftPlaneUI);
	//if (SUCCEEDED(CDXUTIMEEditBox::CreateIMEEditBox(&g_LeftPlaneUI, IDC_EDITBOX2,
	//	L"IME-capable edit control with custom styles. Type and press Enter", 20, iY += 50, 600, 45, false, &pIMEEdit)))
	//{
	//	g_LeftPlaneUI.AddControl(pIMEEdit);
	//	//pIMEEdit->GetElement(0)->iFont = 1;
	//	//pIMEEdit->GetElement(1)->iFont = 1;
	//	//pIMEEdit->GetElement(9)->iFont = 1;
	//	//pIMEEdit->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB(128, 255, 255, 255));  // Transparent center
	//	//pIMEEdit->SetBorderWidth(7);
	//	//pIMEEdit->SetTextColor(D3DCOLOR_ARGB(255, 64, 64, 64));
	//	//pIMEEdit->SetCaretColor(D3DCOLOR_ARGB(255, 64, 64, 64));
	//	//pIMEEdit->SetSelectedTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	//	//pIMEEdit->SetSelectedBackColor(D3DCOLOR_ARGB(255, 40, 72, 72));
	//}
}


