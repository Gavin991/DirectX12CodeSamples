#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
#include "..\Common\Collada.h"

namespace DirectX12ColladaDaeBase
{

	class Entity3D 
	{
	
	private:
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;

		float width = 10;
		float depth = 10;
		UINT m = 5;
		UINT n = 5;
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

		int m_iEntity3DNumber = 1;
		int m_iStartOffset = 0;

		void Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix);
	public:
		Entity3D();
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber);
		void ClearConstantBuffer();
		//void CreateEntity3DVector();
		void CreateEntity3D(DAE_Collada::Collada *collada);

		void UpdateVertexAndIndexSubresources();

		void UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);

		void CreateBufferView();

		void UpdateEntity3DByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix);

		void DrawEntity3D(int currentFrameIndex);

		void RemoveUselessContent();

		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
			XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
		}

		int GetEntity3DNumber() { return m_iEntity3DNumber; }
	};

	struct VerticeAdapter
	{	
		VerticeAdapter()
		{
			oriIndex = UINT_MAX;
			//uv2index.clear();
			curIndex = UINT_MAX;
			uvIndex = UINT_MAX;
		};
		VerticeAdapter(unsigned int ori, unsigned int uv, unsigned int cur)
		{
			oriIndex = ori;
			//uv2index.clear();
			curIndex = cur;
			uvIndex = uv;
		};

		//std::map<int, int> uv2index;//uv --新的顶点索引
		unsigned int oriIndex;//原来的顶点索引
		unsigned int curIndex;//新分配的顶点索引
		unsigned int uvIndex;//uv的索引
		
		bool operator<( const VerticeAdapter &b) const {

			if (oriIndex < b.oriIndex)
			{
				return true;
			}				
			else if (oriIndex == b.oriIndex) 
			{
				if (uvIndex < b.uvIndex)
				{
					return true;
				}					
				else if (uvIndex ==b.uvIndex)
				{
					if (curIndex < b.curIndex)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}					
			}
			else
			{
				return false;
			}				
		}

	};

	struct classcomp {
		bool operator() (const unsigned int& lhs, const unsigned int& rhs) const
		{
			return lhs < rhs;
		}
	};
}