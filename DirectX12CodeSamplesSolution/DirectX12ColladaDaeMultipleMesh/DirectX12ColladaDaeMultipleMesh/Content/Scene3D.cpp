#include "pch.h"
#include "Scene3D.h"
#include "..\Common\DirectXHelper.h"
using namespace DirectX;

namespace DirectX12ColladaDaeMultipleMesh
{

	Scene3D::Scene3D()
	{
		
	}
	
	void Scene3D::ClearScene3DConstantBuffer()
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->ClearConstantBuffer();
		}	
	}

	void Scene3D::RemoveScene3DUselessContent()
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->RemoveUselessContent();
		}
	}

	void Scene3D::SetScene3DViewMatrix(DirectX::XMMATRIX modelMatrix, DirectX::XMMATRIX viewMatrix)
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->SetModelViewMatrix(modelMatrix, viewMatrix);
		}
	}

	void Scene3D::UpdateScene3DSubresources()
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateVertexAndIndexSubresources();
		}
	}

	void Scene3D::UpLoadScene3DConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager)
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpLoadVextureConstantBuffer(cbvHeapManager);
		}
	}

	void Scene3D::CreateScene3DBufferView()
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->CreateBufferView();
		}
	}

	int Scene3D::GetAllEnity3DNumber()
	{
		int total = 0;
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			total+=it->get()->GetEntity3DNumber();
		}
		return total;
	}

	void Scene3D::UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager, UINT rootParameterIndex)
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateAndDraw(cbvHeapManager, currentFrameIndex, curMatrix, textureManager, rootParameterIndex);			
		}
	}

	void Scene3D::GetAllImage()
	{
		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			//it->get()->GetImageName();
			//imageSet.insert(it->get()->GetImageName());
			GetEnity3DImage(it->get());
		}
		//std::set<std::wstring> imageSet;
	}

	void Scene3D::GetEnity3DImage(Entity3D* entity3D)
	{
		if (!entity3D->GetImageName().empty())
		{
			imageSet.insert(entity3D->GetImageName());
		}	
		for (std::list<std::shared_ptr<Entity3D>>::iterator itSub = entity3D->m_entity3d_list.begin(); itSub != entity3D->m_entity3d_list.end(); ++itSub)
		{			
			GetEnity3DImage(itSub->get());
		}
	}

	void Scene3D::CreatScene3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, const DAE_Collada::Collada *collada)
	{
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		m_entity3d_list.clear();

		name = collada->m_VisualSceneLibrary.m_VisualScene->m_Name;

		
		vector<std::unique_ptr<DAE_Collada::Node>>::const_iterator iterator = collada->m_VisualSceneLibrary.m_VisualScene->m_nodes.begin();
		for (; iterator != collada->m_VisualSceneLibrary.m_VisualScene->m_nodes.end(); iterator++)
		{
			//std::wstring nodeName = iterator->get()->m_Name;
			std::shared_ptr<Entity3D> tempEntity3D(new Entity3D);
			tempEntity3D->CreateEntity3D(m_pD3dDevice, m_pCommandList, collada, iterator->get());
			//tempEntity3D->SetOriginalMatrix(iterator->get()->m_matrix->m_matrix);
			m_entity3d_list.push_back(tempEntity3D);
		
			
		}
	}
}
