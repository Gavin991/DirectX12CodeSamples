#include "pch.h"
#include "TextureManager.h"
#include <tchar.h> 
#include "..\Common\DirectXHelper.h"

#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"


#include "..\Common\DDSTextureLoader.h"

using namespace DirectX;

namespace DirectX12ColladaDaeBase
{
	TextureManager::TextureManager()
	{
		m_textureMap.clear();
	}

	void TextureManager::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device)
	{
		m_pD3dDevice = device;	
	}

	void TextureManager::LoadTexture(ConstantBufferViewHeapManager *cbvHeapManager, LPCWSTR szFile, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		map<LPCWSTR, shared_ptr<TextureInformationStruct>>::iterator it;
		it = m_textureMap.find(szFile);
		if (it == m_textureMap.end())
		{
			/*TextureInformationStruct tempStruct;*/
			shared_ptr<TextureInformationStruct> tempStruct(new TextureInformationStruct());
		
#pragma region 加载纹理部分
			//LPCWSTR szFile = L"Assets\\pic.jpg";

			DWORD flags = DirectX::CP_FLAGS::CP_FLAGS_NONE;
			ScratchImage image;
			DirectX::LoadFromWICFile(szFile, flags, nullptr, image);

			auto metadata1 = image.GetMetadata();
			auto texture = image.GetPixels();

			CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::
				Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, metadata1.width, metadata1.height, 1, 1);

			DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&tempStruct->m_texture)));

			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tempStruct->m_texture.Get(), 0, 1) + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

			DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&tempStruct->m_pTextureUpload)));

			TCHAR  textrue_name[256];
			_stprintf_s(textrue_name, L"Shader Resource Texture: %s", szFile);			
			tempStruct->m_texture->SetName(textrue_name);
			_stprintf_s(textrue_name, L"Shader Upload Resource: %s", szFile);
			tempStruct->m_pTextureUpload->SetName(textrue_name);

			// Upload the Shader Resource to the GPU.
			{
				// Copy data to the intermediate upload heap and then schedule a copy
				// from the upload heap to the texture.
				D3D12_SUBRESOURCE_DATA textureData = {};
				textureData.pData = texture;//reinterpret_cast<UINT8*>(texture.data());
				textureData.RowPitch = image.GetImages()->rowPitch;
				textureData.SlicePitch = textureData.RowPitch * metadata1.height;

				//tempStruct->m_pCommandList = commandList;

				UpdateSubresources(commandList.Get(), tempStruct->m_texture.Get(), tempStruct->m_pTextureUpload.Get(), 0, 0, 1, &textureData);
				tempStruct->offsetInDescriptors = cbvHeapManager->GetHeapCurrentOffset();
				cbvHeapManager->CreateShaderResourceView(tempStruct->m_texture, textureDesc);
			}

#pragma endregion

			m_textureMap.insert(pair<LPCWSTR, shared_ptr<TextureInformationStruct>>(szFile, tempStruct));
			//TCHAR  sOut[512];
			//_stprintf_s(sOut, L"开始加载图片: %s\n", szFile);
			////_stprintf_s("%d", _T(UINT));
			//OutputDebugString(sOut);
		}
		else
		{
			TCHAR  sOut[512];
			_stprintf_s(sOut, L"重复加载图片: %s\n", szFile);
			//_stprintf_s("%d", _T(UINT));
			OutputDebugString(sOut);
			//m_textureMap.erase(it);
		}		

	}

	void TextureManager::RemoveUselessContent()
	{
		//tempStruct.m_pTextureUpload.Reset();
		////vertexBufferUpload.re
		for (map<LPCWSTR, shared_ptr<TextureInformationStruct>>::iterator it = m_textureMap.begin(); it != m_textureMap.end(); ++it)
			it->second->m_pTextureUpload.Reset();
	}

	void  TextureManager::BindTexture(ConstantBufferViewHeapManager *cbvHeapManager,  LPCWSTR szFile, UINT rootParameterIndex, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		int texturOffsetIndex = m_textureMap[szFile]->offsetInDescriptors;//从0开始
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), texturOffsetIndex, cbvHeapManager->GetCbvDescriptorSize());
		commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHandle);
	}

	void TextureManager::LoadDDsTexture(ConstantBufferViewHeapManager *cbvHeapManager, LPCWSTR szFile, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_SRV_DIMENSION srvType)
	{	
		map<LPCWSTR, shared_ptr<TextureInformationStruct>>::iterator it;
		it = m_textureMap.find(szFile);
		if (it == m_textureMap.end())
		{			
			shared_ptr<TextureInformationStruct> tempStruct(new TextureInformationStruct());

#pragma region 加载纹理部分			
			DirectX::CreateDDSTextureFromFile12(m_pD3dDevice.Get(),
				commandList.Get(), szFile,
				tempStruct->m_texture, tempStruct->m_pTextureUpload);			

			TCHAR  textrue_name[256];
			_stprintf_s(textrue_name, L"Shader Resource Texture: %s", szFile);
			tempStruct->m_texture->SetName(textrue_name);
			_stprintf_s(textrue_name, L"Shader Upload Resource: %s", szFile);
			tempStruct->m_pTextureUpload->SetName(textrue_name);

			tempStruct->offsetInDescriptors = cbvHeapManager->GetHeapCurrentOffset();
			cbvHeapManager->CreateShaderResourceViewDDs(tempStruct->m_texture, srvType);
#pragma endregion
			m_textureMap.insert(pair<LPCWSTR, shared_ptr<TextureInformationStruct>>(szFile, tempStruct));
		}
		else
		{
			TCHAR  sOut[512];
			_stprintf_s(sOut, L"重复加载图片: %s\n", szFile);			
			OutputDebugString(sOut);			
		}

	}


}
