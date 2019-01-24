#include "pch.h"
#include "ConstantPixelShaderBufferInformation.h"
#include "..\Common\DirectXHelper.h"
using namespace DirectX;

namespace DirectX12XAMLGeometry_Shader
{
	ConstantPixelShaderBufferInformation::ConstantPixelShaderBufferInformation()
	{
	}



	void ConstantPixelShaderBufferInformation::PixelConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12Device>	pD3dDevice, const UINT alignedConstantPSBufferSize)
	{

		DX::ThrowIfFailed(pD3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(alignedConstantPSBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantPSBuffer)));

		m_constantPSBuffer->SetName(L"Pixel Shader Constant Buffer");

		// Create constant buffer views to access the upload buffer.
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantPSBuffer->GetGPUVirtualAddress();
		offsetInDescriptors = cbvHeapManager->GetHeapCurrentOffset();
		cbvHeapManager->CreateConstantBufferView(cbvGpuAddress, alignedConstantPSBufferSize);

		// Map the constant buffers.
		DX::ThrowIfFailed(m_constantPSBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantPSBuffer)));
		ZeroMemory(m_mappedConstantPSBuffer, alignedConstantPSBufferSize);
		// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

	}

	void ConstantPixelShaderBufferInformation::ClearConstantBuffer()
	{
		if (m_constantPSBuffer != nullptr)
		{
			m_constantPSBuffer->Unmap(0, nullptr);
		}
		m_mappedConstantPSBuffer = nullptr;
	}

	void  ConstantPixelShaderBufferInformation::BindPSConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList, UINT rootParameterIndex)
	{		
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), offsetInDescriptors, cbvHeapManager->GetCbvDescriptorSize());
		pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHandle);
	}


	void ConstantPixelShaderBufferInformation::Update()
	{
		// Update the ambient color
		m_constantPSBufferData.ambientColor = XMFLOAT4(0.31f, 0.31f, 0.31f, 1.0f); // light black
		m_constantPSBufferData.specularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // red directional red color
		m_constantPSBufferData.lightDirection = XMFLOAT3(m_x, -1.0f, -1.0f); // light in the z direction	
		m_constantPSBufferData.power = 15.0f;
		memcpy(m_mappedConstantPSBuffer, &m_constantPSBufferData, sizeof(m_constantPSBufferData));
	}

}
