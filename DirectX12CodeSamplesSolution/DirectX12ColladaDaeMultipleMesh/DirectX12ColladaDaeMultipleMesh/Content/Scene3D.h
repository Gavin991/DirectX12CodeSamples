#pragma once


#include "..\Common\Collada.h"
#include "Entity3D.h"

namespace DirectX12ColladaDaeMultipleMesh
{


	class Scene3D sealed
	{

	private:
		Microsoft::WRL::ComPtr<ID3D12Device>						m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;
	public:
		Scene3D();
		std::list<std::shared_ptr<Entity3D>> m_entity3d_list;
		std::wstring name;
		void CreatScene3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, const DAE_Collada::Collada *collada);
		void ClearScene3DConstantBuffer();
		void RemoveScene3DUselessContent();
		void SetScene3DViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix);
		void UpdateScene3DSubresources();
		void UpLoadScene3DConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);
		void CreateScene3DBufferView();
		int GetAllEnity3DNumber();
		void UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager, UINT rootParameterIndex);
		void GetEnity3DImage(Entity3D *entiy3D);

		std::set<std::wstring> imageSet;
		void GetAllImage();
	};

}