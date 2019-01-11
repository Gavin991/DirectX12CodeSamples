#pragma once


#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"

namespace DirectX12XAMLCubeMap
{
	class CustomPlane 
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;

		float width = 10;
		float depth = 10;
		UINT m = 5;
		UINT n = 5;
		int m_iIndices = 0;
		const UINT c_AlignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBufferForPlane) + 255) & ~255;

		std::vector<VertexPositionColorForPlane> VertexArray;
		std::vector<unsigned short> indexArray;

		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;

		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		//----
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBufferUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBufferUpload;



		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		ModelViewProjectionConstantBufferForPlane			m_constantBufferData;// ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));;
		UINT8*												m_mappedConstantBuffer;

		UINT												m_cbvDescriptorSize = 0;

		int m_iSphereNumber = 1;
		int m_iStartOffset = 0;

		void Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix);
	public:
		CustomPlane();
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber);
		void ClearConstantBuffer();
		void CreatePlaneVector();


		void UpdateVertexAndIndexSubresources();

		void UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);

		void CreateBufferView();

		void UpdatePlaneByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix);

		void DrawPlane(int currentFrameIndex);

		void RemoveUselessContent();

		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
			XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
		}

		int GetPlaneNumber() { return m_iSphereNumber; }
	};

}