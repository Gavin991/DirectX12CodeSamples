#include "pch.h"
#include "ConstantBufferViewHeapManager.h"
#include "..\Common\DirectXHelper.h"

namespace DirectX12XAMLAlphaBlend
{
	ConstantBufferViewHeapManager::ConstantBufferViewHeapManager()
	{
	}

	void ConstantBufferViewHeapManager::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, int descriptorTotal)
	{
		m_pD3dDevice = d3dDevice;

		// 为常量缓冲区创建描述符堆。
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			//heapDesc.NumDescriptors = DX::c_frameCount+1;//因为了多来一个纹理采样器
			heapDesc.NumDescriptors = descriptorTotal ;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// 此标志指示此描述符堆可以绑定到管道，并且其中包含的描述符可以由根表引用。
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;			
			DX::ThrowIfFailed(m_pD3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));
			NAME_D3D12_OBJECT(m_cbvHeap);
		}
		m_cbvCpuHandle = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();
		m_cbvDescriptorSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_iCurrentOffset = 0;
	}


	//创建顶点/像素/着色器的常量缓存视图，使用到堆描述符
	void ConstantBufferViewHeapManager::CreateConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS &cbvGpuAddress, const UINT alignedConstantBufferSize)
	{
		m_iCurrentOffset++;
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = alignedConstantBufferSize;
		m_pD3dDevice->CreateConstantBufferView(&desc, m_cbvCpuHandle);
		cbvGpuAddress += desc.SizeInBytes;
		m_cbvCpuHandle.Offset(m_cbvDescriptorSize);
	}

	//创建纹理着色的常量缓存缓存视图，使用到堆描述符
	void ConstantBufferViewHeapManager::CreateShaderResourceView(Microsoft::WRL::ComPtr<ID3D12Resource> texture, CD3DX12_RESOURCE_DESC textureDesc)
	{
		m_iCurrentOffset++;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		m_pD3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, m_cbvCpuHandle);
		m_cbvCpuHandle.Offset(m_cbvDescriptorSize);
	}

}