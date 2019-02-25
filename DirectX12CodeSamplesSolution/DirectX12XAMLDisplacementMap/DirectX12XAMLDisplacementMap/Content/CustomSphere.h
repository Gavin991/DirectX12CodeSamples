#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
namespace DirectX12XAMLDisplacementMap
{
	//�Զ�������
	class CustomSphere
	{
	private:
		// �����DirectX12�豸ָ�롣		
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		// ����������б�		
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;		
		//����İ뾶
		float m_fRadius = 0.5;
		//���ά�ȷ���ȷ���
		int m_iSlice = 80;
		//��ľ��ȷ���ȷ���
		int m_iStack = 80;
		//�������������
		int m_iIndices=0;
		// ������������С���붼�� 256 �ֽڵ���������
		const UINT c_AlignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;
		//����Ķ�������
		std::vector<VertexPositionColor> VertexArray;
		//�������������
		std::vector<unsigned short> indexArray;
		//����Ķ��㻺��
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		//�������������
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		//����Ķ��㻺����ͼ
		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		//���������������ͼ
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;
		//���ص�GPU�Ķ��㻺����Դ
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBufferUpload;
		//���ص�GPU������������Դ
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBufferUpload;
		//������ɫ���ĳ���������
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		//Ҫ�ϴ���������ɫ���ĳ�������
		ModelViewProjectionConstantBuffer					m_constantBufferData;// ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));;
		//ӳ�䶥����ɫ���Ļ���
		UINT8*												m_mappedConstantBuffer;		
		//��������Ķ���������Ԫ��С
		UINT												m_cbvDescriptorSize = 0;	
		//�м�������
		int m_iSphereNumber = 1;
		//�ڶ������������е������ڵ���ʼƫ����
		int m_iStartOffset = 0;
		//������ľ���
		void Update(int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos);		

		////���潻��ѡ��
		bool m_isAround = false;
		bool m_isRotate = true;
		float	m_angle;
		

		//��ķ���ƫ����X�� { 0.0f, 0.7f, 1.5f, 0.0f }
		float m_fLightX = -1;
		float m_fLightY = -1;
		float m_fLightZ = -1;
		float m_fCameraX = 0.0f;
		float m_fCameraY = 0.7f;
		float m_fCameraZ = 1.5f;

		//XMMATRIX M_curPosMatrix;
		//XMMATRIX M_curRotateMatrix;
	public:
		CustomSphere();
		//��ʼ����������
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber);
		//���������������
		void ClearConstantBuffer();
		//������Ķ������ݺ���������
		void CreateSphereVector();
		//����Ķ������ݺ������������ص�GPU
		void UpdateVertexAndIndexSubresources();	
		//����������ɫ����ʹ�õĳ���������ͼ�ͻ���ӳ��
		void UpLoadVextureConstantBuffer( ConstantBufferViewHeapManager *cbvHeapManager);	
		//������Ķ�����ͼ�����������ͼ
		void CreateBufferView();	
		//��������ľ�����Ϣ��ƫ�Ƶ���Ӧ�Ķ�������
		void UpdateSphereByIndex(ConstantBufferViewHeapManager *cbvHeapManager,int sphereIndex, int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos);
		//��Ⱦһ����
		void DrawSphere();
		//���ص�GPU��ɺ󣬿����ͷŵ���Դ
		void RemoveUselessContent();		
		//���ó�ʼ�ľ�����Ϣ
		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
			XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
		}
		//�õ�Ҫ��Ⱦ���ٸ�����
		int GetSphereNumber() {			return m_iSphereNumber;		}
	
		//����ʱ��Ƭ
		void UpdateElapsedSeconds(float elapsedTime);
		void SetRotateEnable();
		void  HandleLightPosChanged(int dir,float pos)
		{ 
			if (dir == 0)m_fLightX = pos;
			if (dir == 1)m_fLightY = pos;
			if (dir == 2)m_fLightZ = pos;
		}
		
		void HandleCameraXChanged(float x) { m_fCameraX = x; }
		void HandleCameraYChanged(float y) { m_fCameraY = y; }
		//	float m_fCameraX = 0.0f;
		//float m_fCameraY = 0.7f;
		//float m_fCameraZ = 1.5f;
	};
}
