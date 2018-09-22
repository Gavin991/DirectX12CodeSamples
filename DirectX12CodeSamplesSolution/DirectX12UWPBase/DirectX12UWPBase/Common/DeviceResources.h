#pragma once
//为拥有 DeviceResources 的应用程序提供一个界面，以在丢失或创建设备时收到通知。

namespace DX
{
	static const UINT c_frameCount = 3;		// 使用三重缓冲。

	// 控制所有 DirectX 设备资源。
	class DeviceResources
	{
	public:
		DeviceResources();
		void SetWindow(Windows::UI::Core::CoreWindow^ window);
		void SetLogicalSize(Windows::Foundation::Size logicalSize);
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
		void SetDpi(float dpi);
		void ValidateDevice();
		void Present();
		void WaitForGpu();

		// 呈现器目标的大小，以像素为单位。
		Windows::Foundation::Size	GetOutputSize() const				{ return m_outputSize; }

		// 呈现器目标的大小，以 dip 为单位。
		Windows::Foundation::Size	GetLogicalSize() const				{ return m_logicalSize; }

		float						GetDpi() const						{ return m_effectiveDpi; }
		bool						IsDeviceRemoved() const				{ return m_deviceRemoved; }

		// D3D 访问器。
		ID3D12Device*				GetD3DDevice() const				{ return m_d3dDevice.Get(); }
		IDXGISwapChain3*			GetSwapChain() const				{ return m_swapChain.Get(); }
		ID3D12Resource*				GetRenderTarget() const				{ return m_renderTargets[m_currentFrame].Get(); }
		ID3D12Resource*				GetDepthStencil() const				{ return m_depthStencil.Get(); }
		ID3D12CommandQueue*			GetCommandQueue() const				{ return m_commandQueue.Get(); }
		ID3D12CommandAllocator*		GetCommandAllocator() const			{ return m_commandAllocators[m_currentFrame].Get(); }
		D3D12_VIEWPORT				GetScreenViewport() const			{ return m_screenViewport; }
		DirectX::XMFLOAT4X4			GetOrientationTransform3D() const	{ return m_orientationTransform3D; }
		UINT						GetCurrentFrameIndex() const		{ return m_currentFrame; }

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrame, m_rtvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

	private:
		void CreateDeviceIndependentResources();
		void CreateDeviceResources();
		void CreateWindowSizeDependentResources();
		void UpdateRenderTargetSize();
		void MoveToNextFrame();
		DXGI_MODE_ROTATION ComputeDisplayRotation();
		void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);

		UINT											m_currentFrame;

		// Direct3D 对象。
		Microsoft::WRL::ComPtr<ID3D12Device>			m_d3dDevice;//
		Microsoft::WRL::ComPtr<IDXGIFactory4>			m_dxgiFactory;//
		Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;//
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderTargets[c_frameCount];// mSwapChainBuffer
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_depthStencil;//mDepthStencilBuffer
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;//
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;//
		UINT											m_rtvDescriptorSize;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_commandQueue;//
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	m_commandAllocators[c_frameCount];
		D3D12_VIEWPORT									m_screenViewport;//
		bool											m_deviceRemoved;

		// CPU/GPU 同步。
		Microsoft::WRL::ComPtr<ID3D12Fence>				m_fence;//
		UINT64											m_fenceValues[c_frameCount];
		HANDLE											m_fenceEvent;

		// 对窗口的缓存引用。
		Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;

		// 缓存的设备属性。
		Windows::Foundation::Size						m_d3dRenderTargetSize;
		Windows::Foundation::Size						m_outputSize;
		Windows::Foundation::Size						m_logicalSize;
		Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
		float											m_dpi;

		// 这是将向应用传回的 DPI。它考虑了应用是否支持高分辨率屏幕。
		float											m_effectiveDpi;

		// 用于显示方向的转换。
		DirectX::XMFLOAT4X4								m_orientationTransform3D;
	};
}
