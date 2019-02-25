#pragma once

#include <map>  
#include <string>  
using namespace std;

namespace DirectX12XAMLDisplacementMap
{
	//��������Ķ�����������������������/����/��ɫ�ĳ�������Ķѡ��Լ�����������Ķ�
	class ConstantBufferViewHeapManager 
	{
	private:
		// �����DirectX12�豸ָ�롣			
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		//CPU���������
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_cbvCpuHandle;
		//��������
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;

		//��ǰ����������ַ��ƫ����
		UINT m_iCurrentOffset = 0;
		//���������ĵ�Ԫ��С
		UINT m_cbvDescriptorSize = 0;

	public:
		ConstantBufferViewHeapManager();	
		//��ʼ�����������������
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,int descriptorTotal);
		//��ȡ��ǰ����������ַ��ƫ����
		inline UINT GetHeapCurrentOffset() {			return m_iCurrentOffset;		}
		//��ȡ���������ĵ�Ԫ��С
		inline  UINT GetCbvDescriptorSize() {			return m_cbvDescriptorSize;		}	
		//��ȡ����������ָ��
		inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCbvHeap(){return m_cbvHeap;}
		//��������/����/��ɫ���ĳ���������ͼ��ʹ�õ���������
		 void CreateConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS &cbvGpuAddress, const UINT alignedConstantBufferSize);
		//����������ɫ�ĳ������滺����ͼ��ʹ�õ���������
		 void CreateShaderResourceView(Microsoft::WRL::ComPtr<ID3D12Resource> texture, CD3DX12_RESOURCE_DESC textureDesc);	
		 void  CreateShaderResourceViewDDs(Microsoft::WRL::ComPtr<ID3D12Resource> texture, D3D12_SRV_DIMENSION srvType = D3D12_SRV_DIMENSION_TEXTURE2D);
		//�õ�������������GPU����ʼ��ַ
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart()
		{
			return m_cbvHeap->GetGPUDescriptorHandleForHeapStart();
		}
	};
}

