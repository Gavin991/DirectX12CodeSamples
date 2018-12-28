#pragma once
#include "ShaderStructures.h"
#include "ConstantBufferViewHeapManager.h"
namespace DirectX12UWPTwoPipelineState
{
	//像素着色常量缓存
	class ConstantPixelShaderBufferInformation
	{
	private:
		//常量缓存指针
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantPSBuffer;
		//常量缓存的数据结构
		ConstantPSBuffer									m_constantPSBufferData;
		//常量缓存的映射指针
		UINT8*												m_mappedConstantPSBuffer;
		//此常量缓存所对应的描述符的偏移量
		int offsetInDescriptors;

	public:
		ConstantPixelShaderBufferInformation();
		//创建并映射常量缓存视图
		void PixelConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12Device>	pD3dDevice, const UINT alignedConstantPSBufferSize);
		//取消映射和释放资源
		void ClearConstantBuffer();
		//将常量缓存所对应的描述符绑定到渲染关系
		void BindPSConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList, UINT rootParameterIndex);
		//设置常量缓存信息
		void Update();
	};

}