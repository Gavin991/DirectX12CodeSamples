#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace DirectX12UWPBase;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// DirectX 12 应用程序模板记录在 http://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409 处

// 主函数仅用于初始化我们的 IFrameworkView 类。
//IFrameworkView表示应用程序的视图提供程序
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	OutputDebugString(L"准备--main----1\n");
	//ref new 分配的类型的实例的内存会自动释放。 聚合关键字分配在对象无法访问时进行垃圾回收以及返回已分配对象的句柄 (^) 的类型的实例。
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	//CoreApplication是个Win10系统的静态类，用于处理状态更改，管理窗口，以及各种UI框架的整合。	
	CoreApplication::Run(direct3DApplicationSource);
	OutputDebugString(L"准备--main----2\n");
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// 创建 IFrameworkView 时调用的第一个方法。
void App::Initialize(CoreApplicationView^ applicationView)
{
	// 注册应用生命周期的事件处理程序。此示例包括 Activated，因此我们
	// 可激活 CoreWindow 并开始在窗口上渲染。
	OutputDebugString(L"准备---Initialize---3\n");
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// 创建(或重新创建) CoreWindow 对象时调用。
//CoreWindow表示具有输入事件和基本用户界面行为的 Windows 应用商店应用程序。
// CoreApplicationView表示应用程序窗口及其线程。
void App::SetWindow(CoreWindow^ window)
{
	OutputDebugString(L"准备--SetWindow----4\n");
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);
}

// 初始化场景资源或加载之前保存的应用状态。
void App::Load(Platform::String^ entryPoint)
{
	OutputDebugString(L"准备---Load---5\n");
	OutputDebugString(entryPoint->Data()		);
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<DirectX12UWPBaseMain>(new DirectX12UWPBaseMain());
	}
}

// 将在窗口处于活动状态后调用此方法。
void App::Run()
{
	OutputDebugString(L"准备---Run---6\n");
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			auto commandQueue = GetDeviceResources()->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				m_main->Update();
			}
			PIXEndEvent(commandQueue);

			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (m_main->Render())
				{
					GetDeviceResources()->Present();
				}
			}
			PIXEndEvent(commandQueue);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// IFrameworkView 所必需的。
// 终止事件不会导致调用 Uninitialize。如果在应用在前台运行时销毁 IFrameworkView
// 类，则将调用该方法。
void App::Uninitialize()
{
	OutputDebugString(L"准备---Uninitialize---7\n");
}

// 应用程序生命周期事件处理程序。

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	OutputDebugString(L"准备----OnActivated--8\n");
	// Run() 在 CoreWindow 激活前将不会开始。
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	OutputDebugString(L"准备----OnSuspending--9\n");
	// 在请求延期后异步保存应用状态。保留延期
	// 表示应用程序正忙于执行挂起操作。
	// 请注意，延期不是无限期的。在大约五秒后，
	// 将强制应用退出。
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// TODO: 在此处插入代码。
		m_main->OnSuspending();

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// 还原在挂起时卸载的任何数据或状态。默认情况下，
	// 在从挂起中恢复时，数据和状态会持续保留。请注意，
	// 如果之前已终止应用，则不会发生此事件。

	// TODO: 在此处插入代码。
	OutputDebugString(L"准备----OnResuming--10\n");
	m_main->OnResuming();
}

// 窗口事件处理程序。

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	OutputDebugString(L"准备----OnWindowSizeChanged--11\n");
	GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	OutputDebugString(L"准备----OnVisibilityChanged--12\n");
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	OutputDebugString(L"准备----OnWindowClosed--13\n");
	m_windowClosed = true;
}

// DisplayInformation 事件处理程序。

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// 注意: 在此处检索到的 LogicalDpi 值可能与应用的有效 DPI 不匹配
	// 如果正在针对高分辨率设备对它进行缩放。在 DeviceResources 上设置 DPI 后，
	// 应始终使用 GetDpi 方法进行检索。
	// 有关详细信息，请参阅 DeviceResources.cpp。
	OutputDebugString(L"准备----OnDpiChanged--14\n");
	GetDeviceResources()->SetDpi(sender->LogicalDpi);
	m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	OutputDebugString(L"准备----OnOrientationChanged--15\n");
	GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	OutputDebugString(L"准备----OnDisplayContentsInvalidated--16\n");
	GetDeviceResources()->ValidateDevice();
}

std::shared_ptr<DX::DeviceResources> App::GetDeviceResources()
{
	//OutputDebugString(L"准备----GetDeviceResources--17\n");

	if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	{
		// 必须先释放对现有 D3D 设备的所有引用，然后才能
		// 创建新设备。

		m_deviceResources = nullptr;
		m_main->OnDeviceRemoved();
	}

	if (m_deviceResources == nullptr)
	{
		m_deviceResources = std::make_shared<DX::DeviceResources>();
		m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		m_main->CreateRenderers(m_deviceResources);
	}
	return m_deviceResources;
}
