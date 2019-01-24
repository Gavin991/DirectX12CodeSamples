#include "pch.h"
#include "FbxScene3D.h"
#include "..\Common\DirectXHelper.h"
using namespace DirectX;

namespace DirectX12FbxBase
{

	FbxScene3D::FbxScene3D()
	{
		
	}
	
	void FbxScene3D::ClearFbxScene3DConstantBuffer()
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->ClearConstantBuffer();
		}	
	}

	void FbxScene3D::RemoveFbxScene3DUselessContent()
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->RemoveUselessContent();
		}
	}

	void FbxScene3D::SetFbxScene3DViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->SetModelViewMatrix(modelMatrix, viewMatrix);
		}
	}

	void FbxScene3D::UpdateFbxScene3DSubresources()
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateVertexAndIndexSubresources();
		}
	}

	void FbxScene3D::UpLoadFbxScene3DConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager)
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpLoadVextureConstantBuffer(cbvHeapManager);
		}
	}

	void FbxScene3D::CreateFbxScene3DBufferView()
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->CreateBufferView();
		}
	}

	int FbxScene3D::GetAllEnity3DNumber()
	{
		int total = 0;
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			total+=it->get()->GetFbxEntity3DNumber();
		}
		return total;
	}

	void FbxScene3D::UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager, UINT rootParameterIndex)
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateAndDraw(cbvHeapManager, currentFrameIndex, curMatrix, textureManager, rootParameterIndex);			
		}
	}

	void FbxScene3D::GetAllImage()
	{
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			//it->get()->GetImageName();
			//imageSet.insert(it->get()->GetImageName());
			GetEnity3DImage(it->get());
		}
		//std::set<std::wstring> imageSet;
	}

	void FbxScene3D::GetEnity3DImage(FbxEntity3D* entity3D)
	{
		if (!entity3D->GetImageName().empty())
		{
			imageSet.insert(entity3D->GetImageName());
		}	
		for (std::list<std::shared_ptr<FbxEntity3D>>::iterator itSub = entity3D->m_fbxEntity3D_list.begin(); itSub != entity3D->m_fbxEntity3D_list.end(); ++itSub)
		{			
			GetEnity3DImage(itSub->get());
		}
	}

	void FbxScene3D::CreatFbxScene3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, FbxNode* pRootNode)
	{
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		m_entity3d_list.clear();

		FbxString lString;

		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.
		// Bake mesh as VBO(vertex buffer object) into GPU.
		FbxNodeAttribute* lNodeAttribute = pRootNode->GetNodeAttribute();
		lString = pRootNode->GetName();
		//FbxAMatrix& lLocalTransform = pRootNode->EvaluateLocalTransform();



		const int lChildCount = pRootNode->GetChildCount();
		for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
		{
			std::shared_ptr<FbxEntity3D> tempFbxEntity3D(new FbxEntity3D);
			tempFbxEntity3D->CreateFbxEntity3D(m_pD3dDevice, m_pCommandList, pRootNode->GetChild(lChildIndex));
			m_entity3d_list.push_back(tempFbxEntity3D);
		}


		//name = collada->m_VisualSceneLibrary.m_VisualScene->m_Name;

		//
		//vector<std::unique_ptr<DAE_Collada::Node>>::const_iterator iterator = collada->m_VisualSceneLibrary.m_VisualScene->m_nodes.begin();
		//for (; iterator != collada->m_VisualSceneLibrary.m_VisualScene->m_nodes.end(); iterator++)
		//{
		//	//std::wstring nodeName = iterator->get()->m_Name;
		//	std::shared_ptr<Entity3D> tempEntity3D(new Entity3D);
		//	tempEntity3D->CreateEntity3D(m_pD3dDevice, m_pCommandList, collada, iterator->get());
		//	//tempEntity3D->SetOriginalMatrix(iterator->get()->m_matrix->m_matrix);
		//	m_entity3d_list.push_back(tempEntity3D);			

		//	
		//}
	}
}
