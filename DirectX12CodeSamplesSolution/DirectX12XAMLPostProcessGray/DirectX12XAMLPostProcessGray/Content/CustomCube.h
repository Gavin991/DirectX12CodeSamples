#pragma once


#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"

namespace DirectX12XAMLPostProcessGray
{
	class CustomCube
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;

		float m_fWidth = 1;
		float m_fHeight =1;
		float m_fdepth = 1;
		int m_iIndices = 0;

		const UINT c_AlignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		std::vector<VertexPositionColor> VertexArray;
		std::vector<unsigned short> indexArray;

		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;

		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		//----
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBufferUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBufferUpload;



		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		ModelViewProjectionConstantBuffer					m_constantBufferData;// ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));;
		UINT8*												m_mappedConstantBuffer;

		UINT												m_cbvDescriptorSize = 0;

		int m_iSphereNumber = 1;
		int m_iStartOffset = 0;

		void Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix);

	public:
		CustomCube();

		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber);

		void ClearConstantBuffer();

		void CreateCubeVector();

		void UpdateVertexAndIndexSubresources();

		void UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);

		void CreateBufferView();

		void UpdateCubeByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix);

		void DrawCube(int currentFrameIndex);

		void RemoveUselessContent();

		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
			XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
		}

		int GetCubeNumber() { return m_iSphereNumber; }
	};
}

