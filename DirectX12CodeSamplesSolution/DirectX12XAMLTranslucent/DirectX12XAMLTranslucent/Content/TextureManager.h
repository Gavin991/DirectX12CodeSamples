#pragma once

#include <list>
#include <map>  
#include <string>  
using namespace std;

#include "ConstantBufferViewHeapManager.h"

namespace DirectX12XAMLTranslucent
{
	//纹理资源管理器
	class TextureManager 
	{
	private:
		//一张纹理资源
		struct TextureInformationStruct
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;			
			Microsoft::WRL::ComPtr<ID3D12Resource> m_pTextureUpload;	
			//纹理的堆描述符地址的偏移量
			int offsetInDescriptors;
		};
		//纹理资源存放对象
		map<LPCWSTR, shared_ptr<TextureInformationStruct>> m_textureMap;
		// 缓存的DirectX12设备指针。	
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;	
	public:
		TextureManager();
		// 纹理资源管理器。	
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);
		// 从磁盘加载一张纹理。	
		void LoadTexture(ConstantBufferViewHeapManager *cbvHeapManager, LPCWSTR szFile, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);	
		//当纹理上载的GPU后，就可以释放掉的资源。
		void RemoveUselessContent();
		//将纹理绑定到渲染关系。
		void BindTexture(ConstantBufferViewHeapManager *cbvHeapManager, LPCWSTR szFile, UINT rootParameterIndex,  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
	};
}

