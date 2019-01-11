#include "pch.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"
#include <ppltasks.h>
#include <windows.ui.xaml.media.dxinterop.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

namespace DisplayMetrics
{
	// 高分辨率显示可能需要大量 GPU 和电池电源来呈现。
	// 例如，出现以下情况时，高分辨率电话可能会缩短电池使用时间
	// 游戏尝试以全保真度按 60 帧/秒的速度呈现。
	// 跨所有平台和外形规格以全保真度呈现的决定
	// 应当审慎考虑。
	static const bool SupportHighResolutions = false;

	// 用于定义“高分辨率”显示的默认阈值，如果该阈值
	// 超出范围，并且 SupportHighResolutions 为 false，将缩放尺寸
	// 50%。
	static const float DpiThreshold = 96.0f;		// 200% 标准桌面显示。
	static const float WidthThreshold = 1920.0f;	// 1080p 宽。
	static const float HeightThreshold = 1080.0f;	// 1080p 高。
};

// 用于计算屏幕旋转的常量。
namespace ScreenRotation
{
	//0 度 Z 旋转
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 90 度 Z 旋转
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	//  180 度 Z 旋转
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	//270 度 Z 旋转
	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
};

// DeviceResources 的构造函数。
DX::DeviceResources::DeviceResources(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat) :
	m_currentFrame(0),
	m_screenViewport(),
	m_rtvDescriptorSize(0),
	m_fenceEvent(0),
	m_backBufferFormat(backBufferFormat),
	m_depthBufferFormat(depthBufferFormat),
	m_fenceValues{},
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_deviceRemoved(false)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}

// 配置不依赖于 Direct3D 设备的资源。
void DX::DeviceResources::CreateDeviceIndependentResources()
{
}

// 配置 Direct3D 设备，并存储设备句柄和设备上下文。
void DX::DeviceResources::CreateDeviceResources()
{
#if defined(_DEBUG)
	// 如果项目处于调试生成阶段，请通过 SDK 层启用调试。
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(&adapter);

	// 创建 Direct3D 12 API 设备对象
	HRESULT hr = D3D12CreateDevice(
		adapter.Get(),					// 硬件适配器。
		D3D_FEATURE_LEVEL_11_0,			// 此应用可以支持的最低功能级别。
		IID_PPV_ARGS(&m_d3dDevice)		// 返回创建的 Direct3D 设备。
		);

#if defined(_DEBUG)
	if (FAILED(hr))
	{
		// 如果初始化失败，则回退到 WARP 设备。
		// 有关 WARP 的详细信息，请参阅: 
		// http://go.microsoft.com/fwlink/?LinkId=286690

		ComPtr<IDXGIAdapter> warpAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	}
#endif

	DX::ThrowIfFailed(hr);

	// 创建命令队列。
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	NAME_D3D12_OBJECT(m_commandQueue);
	
	//为渲染目标视图和深度模板视图创建描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = c_frameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	NAME_D3D12_OBJECT(m_rtvHeap);

	m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	NAME_D3D12_OBJECT(m_dsvHeap);

	for (UINT n = 0; n < c_frameCount; n++)
	{
		DX::ThrowIfFailed(
			m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n]))
			);
	}

	// 创建同步对象。
	DX::ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValues[m_currentFrame]++;

	m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
}

//  每次更改窗口大小时需要重新创建这些资源。
void DX::DeviceResources::CreateWindowSizeDependentResources()
{
	// 等到以前的所有 GPU 工作完成。
	WaitForGpu();

	// 清除特定于上一窗口大小的内容。
	for (UINT n = 0; n < c_frameCount; n++)
	{
		m_renderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[m_currentFrame];
	}

	UpdateRenderTargetSize();

	// 交换链的宽度和高度必须基于窗口的
	// 面向本机的宽度和高度。如果窗口不在本机
	// 方向，则必须使尺寸反转。
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	UINT backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	UINT backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	if (m_swapChain != nullptr)
	{
		// 如果交换链已存在，请调整其大小。
		HRESULT hr = m_swapChain->ResizeBuffers(c_frameCount, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 如果出于任何原因移除了设备，将需要创建一个新的设备和交换链。
			RemoveDevice();

			// 请勿继续执行此方法。会销毁并重新创建 DeviceResources。
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		//否则，使用与现有 Direct3D 设备相同的适配器新建一个。
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;						// 匹配窗口的大小。
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// 请不要使用多采样。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = c_frameCount;					// 使用三重缓冲最大程度地减小延迟。
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 所有 Windows 通用应用都必须使用 _FLIP_ SwapEffects
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;
		DX::ThrowIfFailed(
			m_dxgiFactory->CreateSwapChainForComposition(
				m_commandQueue.Get(),		//  交换链需要对 DirectX 12 中的命令队列的引用。
				&swapChainDesc,
				nullptr,
				&swapChain
				)
			);

		DX::ThrowIfFailed(swapChain.As(&m_swapChain));

		SetSwapChainOnPanelAsync(m_swapChain.Get());
	}

	// 为交换链设置正确方向，并生成
	// 转换以渲染到旋转交换链。
	// 显式指定 3D 矩阵可以避免舍入误差。

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		throw ref new FailureException();
	}

	DX::ThrowIfFailed(
		m_swapChain->SetRotation(displayRotation)
		);

	// 在交换链上设置反比例。
	DXGI_MATRIX_3X2_F inverseScale = {};
	float dpiScale = m_effectiveDpi / m_dpi;
	inverseScale._11 = 1.0f / (m_compositionScaleX * dpiScale);
	inverseScale._22 = 1.0f / (m_compositionScaleY * dpiScale);

	DX::ThrowIfFailed(
		m_swapChain->SetMatrixTransform(&inverseScale)
		);

	//创建交换链后台缓冲区的呈现目标视图。
	{
		m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < c_frameCount; n++)
		{
			DX::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvDescriptor);
			rtvDescriptor.Offset(m_rtvDescriptorSize);

			WCHAR name[25];
			if (swprintf_s(name, L"m_renderTargets[%u]", n) > 0)
			{
				DX::SetName(m_renderTargets[n].Get(), name);
			}
		}
	}

	// 创建深度模具视图。
	{
		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_depthBufferFormat, 1.0f, 0);

		ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
			));

		NAME_D3D12_OBJECT(m_depthStencil);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	//设置用于确定整个窗口的 3D 渲染视区。
	m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };
}

// 确定呈现器目标的尺寸及其是否将缩小。
void DX::DeviceResources::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	// 为了延长高分辨率设备上的电池使用时间，请呈现到较小的呈现器目标
	// 并允许 GPU 在显示输出时缩放输出。
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold)
	{
		float width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);
		float height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);

		// 当设备纵向显示时，高度大于宽度。请将
		// 较大维度与宽度阈值比较，并将较小维度
		// 与高度阈值进行比较。
		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold)
		{
			// 为了缩放应用，我们更改了有效 DPI。逻辑大小不变。
			m_effectiveDpi /= 2.0f;
		}
	}

	// 计算必要的呈现器目标大小(以像素为单位)。
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// 防止创建大小为零的 DirectX 内容。
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// 这个私有方法将DXGI交换链关联到XAML的SwapChainPanel控件。
IAsyncAction^ DX::DeviceResources::SetSwapChainOnPanelAsync(IDXGISwapChain3* swapChain)
{
	// 当UI改变是，需要回收UI线程。
	return m_swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
	{
		// 得到SwapChainPanel控件的接口。Get backing native interface for SwapChainPanel.
		ComPtr<ISwapChainPanelNative> panelNative;
		DX::ThrowIfFailed(
			reinterpret_cast<IUnknown*>(m_swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
			);

		DX::ThrowIfFailed(panelNative->SetSwapChain(swapChain));
	}, CallbackContext::Any));
}

// 创建(或重新创建) XAML 控件时调用此方法。
void DX::DeviceResources::SetSwapChainPanel(SwapChainPanel^ panel)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_swapChainPanel = panel;
	m_logicalSize = Windows::Foundation::Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_compositionScaleX = panel->CompositionScaleX;
	m_compositionScaleY = panel->CompositionScaleY;
	m_dpi = currentDisplayInformation->LogicalDpi;

	CreateWindowSizeDependentResources();
}

// 在 SizeChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// 在 DpiChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;
		CreateWindowSizeDependentResources();
	}
}

// 在 OrientationChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

//在 CompositionScaleChanged 事件的事件处理程序中调用此方法
void DX::DeviceResources::SetCompositionScale(float compositionScaleX, float compositionScaleY)
{
	if (m_compositionScaleX != compositionScaleX ||
		m_compositionScaleY != compositionScaleY)
	{
		m_compositionScaleX = compositionScaleX;
		m_compositionScaleY = compositionScaleY;
		CreateWindowSizeDependentResources();
	}
}

// 在 DisplayContentsInvalidated 事件的事件处理程序中调用此方法。
void DX::DeviceResources::ValidateDevice()
{
	// 如果默认适配器更改，D3D 设备将不再有效，因为该设备
	// 已创建或该设备已移除。

	// 首先，获取自设备创建以来的默认适配器信息。

	DXGI_ADAPTER_DESC previousDesc;
	{
		ComPtr<IDXGIAdapter1> previousDefaultAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumAdapters1(0, &previousDefaultAdapter));

		DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	// 接下来，获取当前默认适配器的信息。

	DXGI_ADAPTER_DESC currentDesc;
	{
		ComPtr<IDXGIFactory4> currentDxgiFactory;
		DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		ComPtr<IDXGIAdapter1> currentDefaultAdapter;
		DX::ThrowIfFailed(currentDxgiFactory->EnumAdapters1(0, &currentDefaultAdapter));

		DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	// 如果适配器 LUID 不匹配，或者该设备报告它已被移除，
	// 则必须创建新的 D3D 设备。

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		RemoveDevice();
	}
}

// 将交换链的内容显示到屏幕上。
void DX::DeviceResources::Present()
{
	// 第一个参数指示 DXGI 进行阻止直到 VSync，这使应用程序
	// 在下一个 VSync 前进入休眠。这将确保我们不会浪费任何周期渲染
	// 从不会在屏幕上显示的帧。
	HRESULT hr = m_swapChain->Present(1, 0);

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	//如果通过驱动升级或者与驱动设备断开导致设备丢失，则必须重新创建所有设备资源
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		RemoveDevice();
	}
	else
	{
		DX::ThrowIfFailed(hr);

		MoveToNextFrame();
	}
}

// Mark the device as removed and disassociate the DXGI swap chain from the XAML SwapChainPanel.
//将设备标记为已删除，并将DXGI交换链与XAML的SwapChainPanel控件解除关联。
void DX::DeviceResources::RemoveDevice()
{
	m_deviceRemoved = true;

	// There's currently no good way to coordinate when the reference to the DXGI swap chain
	// is removed from the panel. DeviceResources needs to be destroyed and re-created as
	// part of the device recovery process but if the SwapChainPanel is still holding a
	// reference to the DXGI swap chain, a new D3D12 device cannot be created.
	//当从面板中删除对DXGI交换链的引用时，目前没有好的协调处理方式。 
	//DeviceResources作为设备恢复过程的一部分需要销毁和重新创建，
	//但是如果SwapChainPanel仍然持有对DXGI交换链的引用，则不能创建新的D3D12设备。
	SetSwapChainOnPanelAsync(nullptr);
}

//  等待挂起的 GPU 工作完成。
void DX::DeviceResources::WaitForGpu()
{
	//在队列中安排信号命令。
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

	// 等待跨越围栏。
	DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// 对当前帧递增围栏值。
	m_fenceValues[m_currentFrame]++;
}

// 准备呈现下一帧。
void DX::DeviceResources::MoveToNextFrame()
{
	// 在队列中安排信号命令。
	const UINT64 currentFenceValue = m_fenceValues[m_currentFrame];
	HRESULT hr = m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		RemoveDevice();
		return;
	}

	DX::ThrowIfFailed(hr);

	//提高帧索引。
	m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

	//  检查下一帧是否准备好启动。
	if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
	{
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// 为下一帧设置围栏值。
	m_fenceValues[m_currentFrame] = currentFenceValue + 1;
}

// 此方法确定以下两个元素之间的旋转方式: 显示设备的本机方向和
// 当前显示方向。
DXGI_MODE_ROTATION DX::DeviceResources::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// 注意: NativeOrientation 只能为 "Landscape" 或 "Portrait"，即使
	// DisplayOrientations 枚举具有其他值。
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}

// 此方法获取支持 Direct3D 12 的第一个可用硬件适配器。
// 如果找不到此类适配器，则 *ppAdapter 将设置为 nullptr。
void DX::DeviceResources::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// 不要选择基本呈现驱动程序适配器。
			continue;
		}

		// 检查适配器是否支持 Direct3D 12，但不要创建
		// 仍为实际设备。
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
