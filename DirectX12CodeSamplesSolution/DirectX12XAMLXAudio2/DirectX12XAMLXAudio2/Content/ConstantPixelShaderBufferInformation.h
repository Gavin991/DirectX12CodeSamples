#pragma once
#include "ShaderStructures.h"
#include "ConstantBufferViewHeapManager.h"
namespace DirectX12XAMLXAudio2
{
	//������ɫ��������
	class ConstantPixelShaderBufferInformation
	{
	private:
		//��������ָ��
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantPSBuffer;
		//������������ݽṹ
		ConstantPSBuffer									m_constantPSBufferData;
		//���������ӳ��ָ��
		UINT8*												m_mappedConstantPSBuffer;
		//�˳�����������Ӧ����������ƫ����
		int offsetInDescriptors;

		bool m_bCubeMape = false;
		bool m_bNormalMape = false;
		bool m_bParallaxMape = false;

	public:
		ConstantPixelShaderBufferInformation();
		//������ӳ�䳣��������ͼ
		void PixelConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12Device>	pD3dDevice, const UINT alignedConstantPSBufferSize);
		//ȡ��ӳ����ͷ���Դ
		void ClearConstantBuffer();
		//��������������Ӧ���������󶨵���Ⱦ��ϵ
		void BindPSConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList, UINT rootParameterIndex);
		//���ó���������Ϣ
		void Update();
		void CubeMappingEnable();
		void NormalMappingEnable();
		void ParallaxMappingEnable();
	};

}