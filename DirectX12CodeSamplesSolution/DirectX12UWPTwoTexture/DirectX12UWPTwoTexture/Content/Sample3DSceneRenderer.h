#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
#include "CustomSphere.h"
#include "TextureManager.h"
#include "ConstantPixelShaderBufferInformation.h"

namespace DirectX12UWPTwoTexture
{
	// 此示例呈现器实例化一个基本渲染管道。
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Sample3DSceneRenderer();
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void SaveState();

		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }

	private:
		void LoadState();
		void Rotate(float radians);

	private:
		// 常量缓冲区大小必须都是 256 字节的整数倍。
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// 缓存的设备资源指针。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// 立体几何的 Direct3D 资源。
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pipelineState;

		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;
		//UINT												m_cbvDescriptorSize;
		ConstantBufferViewHeapManager m_cbvHeapManager;


		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		//ModelViewProjectionConstantBuffer					    m_constantBufferData;
		//UINT8*												m_mappedConstantBuffer;
		//D3D12_VERTEX_BUFFER_VIEW							    m_vertexBufferView;
		//D3D12_INDEX_BUFFER_VIEW							    m_indexBufferView;
		CustomSphere m_customSphere;
		

		D3D12_RECT											m_scissorRect;

		std::vector<byte>									m_vertexShader;
		std::vector<byte>									m_pixelShader;


		// 用于渲染循环的变量。
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;


		//Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
		TextureManager m_textureManager;


		//// 像素着色器常量缓存，它也必须是256字节对其的
		//// Constant buffers must be 256-byte aligned.
		//static const UINT c_alignedConstantPSBufferSize = (sizeof(ConstantPSBuffer) + 255) & ~255;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantPSBuffer;
		//ConstantPSBuffer									m_constantPSBufferData;
		//UINT8*												m_mappedConstantPSBuffer;
		ConstantPixelShaderBufferInformation m_constantPSBuffer;
		
	};
}

