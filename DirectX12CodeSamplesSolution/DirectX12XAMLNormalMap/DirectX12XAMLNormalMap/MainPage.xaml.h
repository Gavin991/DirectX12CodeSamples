//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"
//+
#include "Common\DeviceResources.h"
#include "DirectX12UWPCubeMain.h"
namespace DirectX12XAMLNormalMap
{
	/// <summary>
	/// 可用于自身或导航至 Frame 内部的空白页。
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		//+
		virtual ~MainPage();
		//+
	private:
		//  m_deviceResources 的专用访问器，防范设备已删除错误。
		std::shared_ptr<DX::DeviceResources> GetDeviceResources();

		// XAML 低级渲染事件处理程序。
		//void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// 窗口事件处理程序。
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// DisplayInformation 事件处理程序。
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// 其他事件处理程序。
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// 在后台工作线程上跟踪我们的独立输入。
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

		// 独立输入处理函数。
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		//+
		// 用于在 XAML 页面背景中呈现 DirectX 内容的资源。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<DirectX12UWPCubeMain> m_main;
		bool m_windowVisible;

		//交互面板的响应事件
		void AroundOnClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void HandleXChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		void OnDispatcherAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher^ sender, Windows::UI::Core::AcceleratorKeyEventArgs^ args);

		float step = 0;
		void ResetButton(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		
		void MovieEnd(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void HandleYChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void HandleZChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void CubeMapeOnClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void NormalMapeOnClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ParallaxMapeOnClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
