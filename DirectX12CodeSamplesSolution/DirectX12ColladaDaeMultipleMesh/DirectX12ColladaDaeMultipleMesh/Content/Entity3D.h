#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
#include "..\Common\Collada.h"
#include "TextureManager.h"

namespace DirectX12ColladaDaeMultipleMesh
{

	class Entity3D 
	{
	
	private:
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;

		unsigned int m_iIndices = 0;
		const UINT c_AlignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBufferForPlane) + 255) & ~255;

		std::vector<VertexPositionColorForPlane> VertexArray;
		std::vector<unsigned short> indexArray;

		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
	//	Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;

		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
	//	D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		//----
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBufferUpload;
	//	Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBufferUpload;



		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		ModelViewProjectionConstantBufferForPlane			m_constantBufferData;// ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));;
		UINT8*												m_mappedConstantBuffer;

		UINT												m_cbvDescriptorSize = 0;

		int m_iEntity3DNumber = 0;
		int m_iStartOffset = 0;
		std::wstring	m_Name;
		void Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix);

		DirectX::XMMATRIX m_matrix;
		
		std::wstring	m_imageStr=L"";
		std::wstring fileStr = L"Assets\\";

	public:
		Entity3D();
		std::list<std::shared_ptr<Entity3D>> m_entity3d_list;
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int number=1);
		void ClearConstantBuffer();
		//void CreateEntity3DVector(); DAE_Collada::Node
		void CreateEntity3D(DAE_Collada::Collada *collada);

		void CreateEntity3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, const DAE_Collada::Collada *collada, DAE_Collada::Node *node);

		void CreateMesh3D(const DAE_Collada::Collada *collada, DAE_Collada::Geometry *geometry);

		void UpdateVertexAndIndexSubresources();

		void UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);

		void CreateBufferView();

		void UpdateEntity3DByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix,  float angle);

		void DrawEntity3D();
		void DrawEntity3D(TextureManager *textureManager,ConstantBufferViewHeapManager *cbvHeapManager, UINT rootParameterIndex);

		void UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager,UINT rootParameterIndex);

		void RemoveUselessContent();

		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			if (m_iEntity3DNumber>0)
			{
				XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
				XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
			}		

			for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
			{
				it->get()->SetModelViewMatrix(modelMatrix, viewMatrix);
			}
		}

		int GetEntity3DNumber() 
		{ 
			int total = 0;
			for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
			{
				total += it->get()->GetEntity3DNumber();
			}
			return total+m_iEntity3DNumber;
		}

		void SetOriginalMatrix(std::vector<float> matrix);

		void SetName(std::wstring name) {	m_Name = name;	}
		std::wstring GetName() { return m_Name ; }
		std::wstring GetImageName() { return m_imageStr; }
	};

}