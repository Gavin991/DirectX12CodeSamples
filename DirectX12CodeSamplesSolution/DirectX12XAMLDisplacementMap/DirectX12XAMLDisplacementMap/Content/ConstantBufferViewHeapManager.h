#pragma once

#include <map>  
#include <string>  
using namespace std;

namespace DirectX12XAMLDisplacementMap
{
	//常量缓存的堆描述符管理、包括创建顶点/像素/着色的常量缓存的堆、以及纹理常量缓存的堆
	class ConstantBufferViewHeapManager 
	{
	private:
		// 缓存的DirectX12设备指针。			
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		//CPU描述符句柄
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_cbvCpuHandle;
		//堆描述符
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;

		//当前堆描述符地址的偏移量
		UINT m_iCurrentOffset = 0;
		//堆描述符的单元大小
		UINT m_cbvDescriptorSize = 0;

	public:
		ConstantBufferViewHeapManager();	
		//初始化堆描述符管理对象
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,int descriptorTotal);
		//获取当前堆描述符地址的偏移量
		inline UINT GetHeapCurrentOffset() {			return m_iCurrentOffset;		}
		//获取堆描述符的单元大小
		inline  UINT GetCbvDescriptorSize() {			return m_cbvDescriptorSize;		}	
		//获取堆描述符的指针
		inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCbvHeap(){return m_cbvHeap;}
		//创建顶点/像素/着色器的常量缓存视图，使用到堆描述符
		 void CreateConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS &cbvGpuAddress, const UINT alignedConstantBufferSize);
		//创建纹理着色的常量缓存缓存视图，使用到堆描述符
		 void CreateShaderResourceView(Microsoft::WRL::ComPtr<ID3D12Resource> texture, CD3DX12_RESOURCE_DESC textureDesc);	
		 void  CreateShaderResourceViewDDs(Microsoft::WRL::ComPtr<ID3D12Resource> texture, D3D12_SRV_DIMENSION srvType = D3D12_SRV_DIMENSION_TEXTURE2D);
		//得到堆描述符的在GPU的起始地址
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart()
		{
			return m_cbvHeap->GetGPUDescriptorHandleForHeapStart();
		}
	};
}

