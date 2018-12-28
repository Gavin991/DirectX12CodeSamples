#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
namespace DirectX12UWPTwoTexture
{
	//自定义球体
	class CustomSphere
	{
	private:
		// 缓存的DirectX12设备指针。		
		Microsoft::WRL::ComPtr<ID3D12Device>			m_pD3dDevice;
		// 缓存的命令列表。		
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;		
		//球体的半径
		float m_fRadius = 0.5;
		//球的维度方向等分数
		int m_iSlice = 80;
		//球的经度方向等分数
		int m_iStack = 80;
		//球的索引缓冲数
		int m_iIndices=0;
		// 常量缓冲区大小必须都是 256 字节的整数倍。
		const UINT c_AlignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;
		//球体的顶点数据
		std::vector<VertexPositionColor> VertexArray;
		//球体的索引数据
		std::vector<unsigned short> indexArray;
		//球体的顶点缓冲
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		//球体的索引缓冲
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		//球体的顶点缓冲视图
		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		//球体的索引缓冲视图
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;
		//上载到GPU的顶点缓冲资源
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBufferUpload;
		//上载到GPU的索引缓冲资源
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBufferUpload;
		//顶点着色器的常量缓冲区
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		//要上传到顶点着色器的常量数据
		ModelViewProjectionConstantBuffer					m_constantBufferData;// ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));;
		//映射顶点着色器的缓存
		UINT8*												m_mappedConstantBuffer;		
		//常量缓存的堆描述符单元大小
		UINT												m_cbvDescriptorSize = 0;	
		//有几个球体
		int m_iSphereNumber = 1;
		//在堆描述符链表中的球所在的起始偏移量
		int m_iStartOffset = 0;
		//更新球的矩阵
		void Update(int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos);		
	public:
		CustomSphere();
		//初始化基本数据
		void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber);
		//清除常量缓存数据
		void ClearConstantBuffer();
		//创建球的顶点数据和索引数据
		void CreateSphereVector();
		//将球的顶点数据和索引数据上载到GPU
		void UpdateVertexAndIndexSubresources();	
		//创建顶点着色器所使用的常量缓冲视图和缓冲映射
		void UpLoadVextureConstantBuffer( ConstantBufferViewHeapManager *cbvHeapManager);	
		//创建球的顶点视图和球的索引视图
		void CreateBufferView();	
		//更新球体的矩阵信息和偏移到对应的堆描述符
		void UpdateSphereByIndex(ConstantBufferViewHeapManager *cbvHeapManager,int sphereIndex, int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos);
		//渲染一个球
		void DrawSphere();
		//上载到GPU完成后，可以释放的资源
		void RemoveUselessContent();		
		//设置初始的矩阵信息
		void SetModelViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
		{
			XMStoreFloat4x4(&m_constantBufferData.projection, modelMatrix);
			XMStoreFloat4x4(&m_constantBufferData.view, viewMatrix);
		}
		//得到要渲染多少个球体
		int GetSphereNumber() {			return m_iSphereNumber;		}
	
	};
}
