#pragma once



#include "FbxEntity3D.h"

namespace DirectX12FbxBase
{


	class FbxScene3D sealed
	{

	private:
		Microsoft::WRL::ComPtr<ID3D12Device>						m_pD3dDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_pCommandList;
	public:
		FbxScene3D();
		std::list<std::shared_ptr<FbxEntity3D>> m_entity3d_list;
		std::wstring name;
		void CreatFbxScene3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, FbxNode* pRootNode);
		void ClearFbxScene3DConstantBuffer();
		void RemoveFbxScene3DUselessContent();
		void SetFbxScene3DViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix);
		void UpdateFbxScene3DSubresources();
		void UpLoadFbxScene3DConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager);
		void CreateFbxScene3DBufferView();
		int GetAllEnity3DNumber();
		void UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager, UINT rootParameterIndex);
		void GetEnity3DImage(FbxEntity3D *entiy3D);

		std::set<std::wstring> imageSet;
		void GetAllImage();
	};

}