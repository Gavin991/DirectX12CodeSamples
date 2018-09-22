#pragma once

#include "pch.h"
#include "Common\DeviceResources.h"
#include "DirectX12UWPTextureMain.h"

namespace DirectX12UWPTexture
{
	// 我们的应用的主入口点。使用 Windows shell 连接应用并处理应用程序生命周期事件。
	ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
	public:
		App();

		// IFrameworkView 方法。
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	protected:
		// 应用程序生命周期事件处理程序。
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);

		// 窗口事件处理程序。
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

		// DisplayInformation 事件处理程序。
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

	private:
		// m_deviceResources 的专用访问器，防范设备已删除错误。
		std::shared_ptr<DX::DeviceResources> GetDeviceResources();

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<DirectX12UWPTextureMain> m_main;
		bool m_windowClosed;
		bool m_windowVisible;
	};
}

//IFrameworkViewSource为视图提供程序对象定义一个工厂。Direct3DApplicationSource就是这个工厂
//sealed关键字表示类不能充当基类。 几乎所有的ref类都是密封的.
ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

