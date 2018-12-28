//
// MainPage.xaml.cpp
// MainPage 类的实现。
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace DirectX12XamlGeometries;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace concurrency;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
//using namespace Windows::UI::Input;
using Microsoft::WRL::ComPtr;

//“空白页”项模板在 http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 上有介绍

MainPage::MainPage()
{
	InitializeComponent();

	// 注册页面生命周期的事件处理程序。
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &MainPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged +=
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &MainPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &MainPage::OnSwapChainPanelSizeChanged);

	// 注册我们的 SwapChainPanel 以获取独立的输入指针事件
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^)
	{
		//对于指定的设备类型，无论它是在哪个线程上创建的，CoreIndependentInputSource 都将引发指针事件。
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
		);

		//指针事件的寄存器，将在后台线程上引发。
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &MainPage::OnPointerReleased);

		// 一旦发送输入消息，即开始处理它们。
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// 在高优先级的专用后台线程上运行任务。
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<DirectX12UWPCubeMain>(new DirectX12UWPCubeMain());

	//键盘响应事件
	Window::Current->CoreWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<Windows::UI::Core::CoreDispatcher^, Windows::UI::Core::AcceleratorKeyEventArgs^>(this, &MainPage::OnDispatcherAcceleratorKeyActivated);
	//window->Dispatcher.AcceleratorKeyActivated += OnDispatcherAcceleratorKeyActivated; (Windows::UI::Core::CoreDispatcher^ sender, Windows::UI::Core::AcceleratorKeyEventArgs^ args)
}


MainPage::~MainPage()
{
	// 析构时停止渲染和处理事件。
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}


// 窗口事件处理程序。

void MainPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop(GetDeviceResources());
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation 事件处理程序。

void MainPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	//critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// 注意: 在此处检索到的 LogicalDpi 值可能与应用的有效 DPI 不匹配
	// 如果正在针对高分辨率设备对它进行缩放。在 DeviceResources 上设置 DPI 后，
	// 应始终使用 GetDpi 方法进行检索。
	// 有关详细信息，请参阅 DeviceResources.cpp。
	GetDeviceResources()->SetDpi(sender->LogicalDpi);
	m_main->OnWindowSizeChanged();
}

void MainPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->OnWindowSizeChanged();
}

void MainPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	GetDeviceResources()->ValidateDevice();
}

// 在单击应用程序栏按钮时调用。
void MainPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// 使用应用程序栏(如果它适合你的应用程序)。设计应用程序栏，
	// 然后填充事件处理程序(与此类似)。
}

void MainPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// 按下指针时开始跟踪指针移动。
	m_main->StartTracking();
}

void MainPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// 更新指针跟踪代码。
	if (m_main->IsTracking())
	{
		m_main->TrackingUpdate(e->CurrentPoint->Position.X);
	}
}

void MainPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	//// 释放指针时停止跟踪指针移动。
	m_main->StopTracking();
}

void MainPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	GetDeviceResources()->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->OnWindowSizeChanged();
}

void MainPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	GetDeviceResources()->SetLogicalSize(e->NewSize);
	m_main->OnWindowSizeChanged();
}

std::shared_ptr<DX::DeviceResources> MainPage::GetDeviceResources()
{
	if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	{
		// All references to the existing D3D device must be released before a new device
		// can be created.

		m_deviceResources = nullptr;
		m_main->StopRenderLoop();
		m_main->OnDeviceRemoved();

#if defined(_DEBUG)
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	if (m_deviceResources == nullptr)
	{
		m_deviceResources = std::make_shared<DX::DeviceResources>();
		m_deviceResources->SetSwapChainPanel(swapChainPanel);
		m_main->CreateRenderers(m_deviceResources);
		m_main->StartRenderLoop(m_deviceResources);
	}
	return m_deviceResources;
}


void DirectX12XamlGeometries::MainPage::AroundOnClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->HandleAroundCheck();
	soundPlayer->Play();
}


void DirectX12XamlGeometries::MainPage::HandleXChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	float changed_x= (float)e->NewValue;
	if (m_windowVisible)
	{
		m_main->HandleXChanged(changed_x);
	}
	
	
	//value = (int)e.newvalue;
}

void DirectX12XamlGeometries::MainPage::OnDispatcherAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher^ sender, Windows::UI::Core::AcceleratorKeyEventArgs^ args)
{
	if (m_windowVisible)
	{
		if (args->EventType == Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown)
		{
			switch (args->VirtualKey)
			{
			case Windows::System::VirtualKey::Up:
				//m_main->HandleXChanged(-2);
				step -= 0.1f;
				m_main->HandleCameraYChanged(step);
				break;
			case Windows::System::VirtualKey::Down:
				//m_main->HandleXChanged(2);
				step += 0.1f;
				m_main->HandleCameraYChanged(step);
				break;
			case Windows::System::VirtualKey::Left:
				//m_main->HandleXChanged(-2);
				step -= 0.1f;
				m_main->HandleCameraXChanged(step);
				break;
			case Windows::System::VirtualKey::Right:
				//m_main->HandleXChanged(2);
				step += 0.1f;
				m_main->HandleCameraXChanged(step);
				break;
			}
		}
	}
}


void DirectX12XamlGeometries::MainPage::ResetButton(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (m_windowVisible)
	{
		step = 0;
		m_main->HandleCameraYChanged(step);
		m_main->HandleCameraXChanged(step);
		soundPlayer->Play();
	
	}
}

void DirectX12XamlGeometries::MainPage::MovieEnd(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	mediaPlayer->Stop();
	mediaPlayer->Source=nullptr;	
	mp3Player->Play();
}
