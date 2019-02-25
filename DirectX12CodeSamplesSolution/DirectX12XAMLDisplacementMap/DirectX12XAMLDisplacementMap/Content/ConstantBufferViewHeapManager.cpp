#include "pch.h"
#include "ConstantBufferViewHeapManager.h"
#include "..\Common\DirectXHelper.h"

namespace DirectX12XAMLDisplacementMap
{
	ConstantBufferViewHeapManager::ConstantBufferViewHeapManager()
	{
	}

	void ConstantBufferViewHeapManager::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice, int descriptorTotal)
	{
		m_pD3dDevice = d3dDevice;

		// Ϊ���������������������ѡ�
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			//heapDesc.NumDescriptors = DX::c_frameCount+1;//��Ϊ�˶���һ�����������
			heapDesc.NumDescriptors = descriptorTotal ;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// �˱�־ָʾ���������ѿ��԰󶨵��ܵ����������а����������������ɸ������á�
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;			
			DX::ThrowIfFailed(m_pD3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));
			NAME_D3D12_OBJECT(m_cbvHeap);
		}
		m_cbvCpuHandle = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();
		m_cbvDescriptorSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_iCurrentOffset = 0;
	}


	//��������/����/��ɫ���ĳ���������ͼ��ʹ�õ���������
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

	//����������ɫ�ĳ������滺����ͼ��ʹ�õ���������
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

	//����������ɫ�ĳ������滺����ͼ��ʹ�õ���������
	void ConstantBufferViewHeapManager::CreateShaderResourceViewDDs(Microsoft::WRL::ComPtr<ID3D12Resource> texture, D3D12_SRV_DIMENSION srvType)
	{
		m_iCurrentOffset++;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texture->GetDesc().Format;
		srvDesc.ViewDimension = srvType;// D3D12_SRV_DIMENSION_TEXTURECUBE;// D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;

		m_pD3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, m_cbvCpuHandle);
		m_cbvCpuHandle.Offset(m_cbvDescriptorSize);
	}

}