#include "pch.h"
#include "Entity3D.h"
#include "..\Common\DirectXHelper.h"
#include <tchar.h> 

using namespace DirectX;

namespace DirectX12ColladaDaeMultipleMesh
{
	Entity3D::Entity3D()
	{
	}

	void Entity3D::Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix)// float radians, DirectX::XMFLOAT3 offsetPos
	{

		XMStoreFloat4x4(&m_constantBufferData.model, DirectX::XMMatrixTranspose(curMatrix));
		// 更新常量缓冲区资源。
		UINT8* destination = m_mappedConstantBuffer + (currentFrameIndex * c_AlignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

	}

	void Entity3D::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int number)
	{
		assert(number > 0);
		m_iEntity3DNumber = number;
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		ClearConstantBuffer();
	}

	void Entity3D::ClearConstantBuffer()
	{
		if (m_constantBuffer != nullptr)
		{
			m_constantBuffer->Unmap(0, nullptr);
		}
		m_mappedConstantBuffer = nullptr;

		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->ClearConstantBuffer();
		}
	}
	

	void Entity3D::CreateEntity3D(DAE_Collada::Collada *collada)
	{
			VertexArray.clear();
			//indexArray.clear();

			std::vector<DirectX::XMFLOAT3> tempVertex;
			std::vector<DirectX::XMFLOAT2> tempUVArray;

			string::size_type position;

			std::wstring  strVertex;
			unsigned int offsetVertex;
			std::wstring  strUV;
			unsigned int offsetUv;		
			vector<std::unique_ptr<DAE_Collada::Input>>::const_iterator iterator = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_Inputs.begin();
			for (; iterator != collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_Inputs.end(); iterator++)
			{
				std::wstring str_temp = iterator->get()->m_Semantic;
				if (str_temp == L"VERTEX")
				{
					strVertex = iterator->get()->m_Source;
					offsetVertex = _wtoi(iterator->get()->m_Offset.c_str());
				}
				else if (str_temp == L"TEXCOORD"&&iterator->get()->m_Set==L"0")
				{
					strUV = iterator->get()->m_Source;
					offsetUv = _wtoi(iterator->get()->m_Offset.c_str());
				}					
			}

			std::wstring   strVerticesID= collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Vertices->m_ID;
			position = strVertex.find(strVerticesID);
			if (position != strVertex.npos)
			{
				//找到存储顶点的节点
				auto itVerticesInput = std::find_if(
					collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Vertices->m_Inputs.cbegin(),
					collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Vertices->m_Inputs.cend(),
					[](const std::unique_ptr<DAE_Collada::Input> &itemInput) {
					if (itemInput->m_Semantic == L"POSITION")
					{
						return true;
					}
					else
					{
						return false;
					}
				});

				if (*itVerticesInput != nullptr)
				{
					std::wstring strVerticesInputSource = itVerticesInput->get()->m_Source;;
					//处理顶点缓冲
					auto itSource = std::find_if(
						collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Sources.cbegin(),
						collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Sources.cend(),
						[&strVerticesInputSource, &position](const std::unique_ptr<DAE_Collada::Source> &itemSource) {
						position = strVerticesInputSource.find(itemSource->m_ID);
						if (position != strVerticesInputSource.npos)
						{
							return true;
						}
						else
						{
							return false;
						}
					});

					if (*itSource != nullptr)
					{
						DAE_Collada::FloatArray ver_float_array = *itSource->get()->m_Float_Array.get();
						unsigned int array_size = ver_float_array.m_Count;
						unsigned int ver_count = static_cast<unsigned int>(floor(array_size / 3));
						tempVertex.resize(ver_count);
						int verIndex = 0;
						for (unsigned int i = 0; i < ver_float_array.m_Floats.size(); i += 3)
						{
							DirectX::XMFLOAT3 ver_pos;
							ver_pos.x = ver_float_array.m_Floats[i];
							ver_pos.y = ver_float_array.m_Floats[i + 1];
							ver_pos.z = ver_float_array.m_Floats[i + 2];
							tempVertex[verIndex] = ver_pos;
							verIndex++;
						}
					}
				}

				//处理UV
				auto itUV = std::find_if(
					collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Sources.cbegin(),
					collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Sources.cend(),
					[&strUV, &position](const std::unique_ptr<DAE_Collada::Source> &itemSource) {
					position = strUV.find(itemSource->m_ID);
					if (position != strUV.npos)
					{
						return true;
					}
					else
					{
						return false;
					}
				});

				if (*itUV != nullptr)
				{
					DAE_Collada::FloatArray uv_float_array = *itUV->get()->m_Float_Array.get();
					unsigned int ver_count = static_cast<unsigned int>(floor(uv_float_array.m_Count / 2));
					tempUVArray.resize(ver_count);

					int uvIndex = 0;
					for (unsigned int i = 0; i < uv_float_array.m_Floats.size(); i += 2)
					{
						DirectX::XMFLOAT2 uv(uv_float_array.m_Floats[i], 1-uv_float_array.m_Floats[i + 1]);
						tempUVArray[uvIndex] = uv;
						uvIndex++;

					}
				}

				DirectX::XMFLOAT3 color(1.0f, 1.0f, 1.0f);
				int count = 0;
				std::map<unsigned int, std::map<unsigned int, unsigned int>> verAdapter;
				std::map<unsigned int, std::map<unsigned int, unsigned int>>::iterator itVer;
				std::map<unsigned int, unsigned int>::iterator iterUV;
			
				unsigned int p_count = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_Count;
				m_iIndices = p_count * 3;

				VertexArray.resize(m_iIndices);

				int vetextIndex = 0;
				unsigned int tri_source_count = static_cast<unsigned int>( collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_Inputs.size());
				p_count = static_cast<unsigned int>( collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P.size());
				for (unsigned int i = 0; i < p_count; i += tri_source_count*3)
				{
					unsigned int ver_index_01 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex];
					unsigned int uv_index_01 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetUv];
					unsigned int ver_index_02 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex + tri_source_count];
					unsigned int uv_index_02 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetUv + tri_source_count];
					unsigned int ver_index_03 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex + tri_source_count * 2];
					unsigned int uv_index_03 = collada->m_GeometryLibrary.m_Geometries.front()->m_Mesh->m_Triangles.front()->m_P[i + offsetUv + tri_source_count * 2];

					VertexArray[vetextIndex].pos = tempVertex[ver_index_01];
					VertexArray[vetextIndex].uv = tempUVArray[uv_index_01];
					VertexArray[vetextIndex].color = color;
					vetextIndex++;

					VertexArray[vetextIndex].pos = tempVertex[ver_index_03];
					VertexArray[vetextIndex].uv = tempUVArray[uv_index_03];
					VertexArray[vetextIndex].color = color;
					vetextIndex++;

					VertexArray[vetextIndex].pos = tempVertex[ver_index_02];
					VertexArray[vetextIndex].uv = tempUVArray[uv_index_02];
					VertexArray[vetextIndex].color = color;
					vetextIndex++;	

				}


			}

	}

	void Entity3D::CreateMesh3D(const DAE_Collada::Collada *collada, DAE_Collada::Geometry *geometry)
	{
		VertexArray.clear();
		//indexArray.clear();

		std::vector<DirectX::XMFLOAT3> tempVertex;
		std::vector<DirectX::XMFLOAT2> tempUVArray;

		string::size_type position;

		std::wstring  strVertex;
		unsigned int offsetVertex;
		std::wstring  strUV;
		unsigned int offsetUv;
		vector<std::unique_ptr<DAE_Collada::Input>>::const_iterator iterator = geometry->m_Mesh->m_Triangles.front()->m_Inputs.begin();
		for (; iterator != geometry->m_Mesh->m_Triangles.front()->m_Inputs.end(); iterator++)
		{
			std::wstring str_temp = iterator->get()->m_Semantic;
			if (str_temp == L"VERTEX")
			{
				strVertex = iterator->get()->m_Source;
				offsetVertex = _wtoi(iterator->get()->m_Offset.c_str());
			}
			else if (str_temp == L"TEXCOORD"&&iterator->get()->m_Set == L"0")
			{
				strUV = iterator->get()->m_Source;
				offsetUv = _wtoi(iterator->get()->m_Offset.c_str());
			}
		}

		std::wstring   strVerticesID = geometry->m_Mesh->m_Vertices->m_ID;
		position = strVertex.find(strVerticesID);
		if (position != strVertex.npos)
		{
			//找到存储顶点的节点
			auto itVerticesInput = std::find_if(
				geometry->m_Mesh->m_Vertices->m_Inputs.cbegin(),
				geometry->m_Mesh->m_Vertices->m_Inputs.cend(),
				[](const std::unique_ptr<DAE_Collada::Input> &itemInput) {
				if (itemInput->m_Semantic == L"POSITION")
				{
					return true;
				}
				else
				{
					return false;
				}
			});

			if (*itVerticesInput != nullptr)
			{
				std::wstring strVerticesInputSource = itVerticesInput->get()->m_Source;;
				//处理顶点缓冲
				auto itSource = std::find_if(
					geometry->m_Mesh->m_Sources.cbegin(),
					geometry->m_Mesh->m_Sources.cend(),
					[&strVerticesInputSource, &position](const std::unique_ptr<DAE_Collada::Source> &itemSource) {
					position = strVerticesInputSource.find(itemSource->m_ID);
					if (position != strVerticesInputSource.npos)
					{
						return true;
					}
					else
					{
						return false;
					}
				});

				if (*itSource != nullptr)
				{
					DAE_Collada::FloatArray ver_float_array = *itSource->get()->m_Float_Array.get();
					unsigned int array_size = ver_float_array.m_Count;
					unsigned int ver_count = static_cast<unsigned int>(floor(array_size / 3));
					tempVertex.resize(ver_count);
					int verIndex = 0;
					for (unsigned int i = 0; i < ver_float_array.m_Floats.size(); i += 3)
					{
						DirectX::XMFLOAT3 ver_pos;
						ver_pos.x = ver_float_array.m_Floats[i];
						ver_pos.y = ver_float_array.m_Floats[i + 1];
						ver_pos.z = ver_float_array.m_Floats[i + 2];
						tempVertex[verIndex] = ver_pos;
						verIndex++;
					}
				}
			}

			//处理UV
			auto itUV = std::find_if(
				geometry->m_Mesh->m_Sources.cbegin(),
				geometry->m_Mesh->m_Sources.cend(),
				[&strUV, &position](const std::unique_ptr<DAE_Collada::Source> &itemSource) {
				position = strUV.find(itemSource->m_ID);
				if (position != strUV.npos)
				{
					return true;
				}
				else
				{
					return false;
				}
			});

			if (*itUV != nullptr)
			{
				DAE_Collada::FloatArray uv_float_array = *itUV->get()->m_Float_Array.get();
				unsigned int ver_count = static_cast<unsigned int>(floor(uv_float_array.m_Count / 2));
				tempUVArray.resize(ver_count);

				int uvIndex = 0;
				for (unsigned int i = 0; i < uv_float_array.m_Floats.size(); i += 2)
				{
					DirectX::XMFLOAT2 uv(uv_float_array.m_Floats[i], 1 - uv_float_array.m_Floats[i + 1]);
					tempUVArray[uvIndex] = uv;
					uvIndex++;

				}
			}

			DirectX::XMFLOAT3 color(1.0f, 1.0f, 1.0f);
			int count = 0;
			std::map<unsigned int, std::map<unsigned int, unsigned int>> verAdapter;
			std::map<unsigned int, std::map<unsigned int, unsigned int>>::iterator itVer;
			std::map<unsigned int, unsigned int>::iterator iterUV;
			
			unsigned int p_count = geometry->m_Mesh->m_Triangles.front()->m_Count;
			m_iIndices = p_count * 3;

			VertexArray.resize(m_iIndices);

			int vetextIndex = 0;
			unsigned int tri_source_count = static_cast<unsigned int>(geometry->m_Mesh->m_Triangles.front()->m_Inputs.size());
			p_count = static_cast<unsigned int>(geometry->m_Mesh->m_Triangles.front()->m_P.size());
			for (unsigned int i = 0; i < p_count; i += tri_source_count * 3)
			{
				unsigned int ver_index_01 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex];
				unsigned int uv_index_01 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetUv];
				unsigned int ver_index_02 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex + tri_source_count];
				unsigned int uv_index_02 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetUv + tri_source_count];
				unsigned int ver_index_03 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetVertex + tri_source_count * 2];
				unsigned int uv_index_03 = geometry->m_Mesh->m_Triangles.front()->m_P[i + offsetUv + tri_source_count * 2];

				VertexArray[vetextIndex].pos = tempVertex[ver_index_01];
				VertexArray[vetextIndex].uv = tempUVArray[uv_index_01];
				VertexArray[vetextIndex].color = color;
				vetextIndex++;

				VertexArray[vetextIndex].pos = tempVertex[ver_index_03];
				VertexArray[vetextIndex].uv = tempUVArray[uv_index_03];
				VertexArray[vetextIndex].color = color;
				vetextIndex++;

				VertexArray[vetextIndex].pos = tempVertex[ver_index_02];
				VertexArray[vetextIndex].uv = tempUVArray[uv_index_02];
				VertexArray[vetextIndex].color = color;
				vetextIndex++;

			}


		}
	}

	void Entity3D::CreateEntity3D(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, const DAE_Collada::Collada *collada, DAE_Collada::Node *node)
	{
		m_entity3d_list.clear();
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		m_iEntity3DNumber = 0;
		SetName(node->m_Name);
		SetOriginalMatrix(node->m_matrix->m_matrix);
		if (node->m_InstanceGeometry)
		{		
			m_iEntity3DNumber = 1;			
			std::wstring	symbol= node->m_InstanceGeometry->m_material->m_symbol;
			string::size_type position;
			
			//查找Material
			auto itMaterial = std::find_if(
				collada->m_MaterialLibrary.m_materials.begin(),
				collada->m_MaterialLibrary.m_materials.cend(),
				[&symbol, &position](const std::unique_ptr<DAE_Collada::Material> &itemMaterial) {
				position = symbol.find(itemMaterial->m_Name);
				if (position != symbol.npos)
				{
					return true;
				}
				else
				{
					return false;
				}
			});

			if (*itMaterial != nullptr)
			{
				//查找Effect
				std::wstring	effect_url =	itMaterial->get()->m_InitFrom->m_url;
				auto itEffect = std::find_if(
					collada->m_effectLibrary.m_effect.begin(),
					collada->m_effectLibrary.m_effect.cend(),
					[&effect_url, &position](const std::unique_ptr<DAE_Collada::Effect> &itemEffect) {
					position = effect_url.find(itemEffect->m_Name);
					if (position != effect_url.npos)
					{
						return true;
					}
					else
					{
						return false;
					}
				});	

				if (*itEffect != nullptr)
				{
					std::wstring	texture_str = itEffect->get()->m_texture->m_texture;
				

					//查找贴图
					auto itImage = std::find_if(
						collada->m_ImageLibrary.m_images.begin(),
						collada->m_ImageLibrary.m_images.cend(),
						[&texture_str, &position](const std::unique_ptr<DAE_Collada::Image> &itemImage) {
						position = texture_str.find(itemImage->m_Name);
						if (position != texture_str.npos)
						{
							return true;
						}
						else
						{
							return false;
						}
					});

					if (*itImage != nullptr)
					{
						std::wstring	image_str = itImage->get()->m_InitFrom->m_path;
						std::string::size_type found = image_str.find_last_of(L"/\\");
						m_imageStr =image_str.substr(found + 1);						
					}//-------------if (*itImage != nullptr)----------------end

				}//-------------if (*itEffect != nullptr)----------------end
			}//-------------if (*itMaterial != nullptr)----------------end


			std::wstring	geometry_url_str = node->m_InstanceGeometry->m_url;
			//查找网格
			auto itgeometry = std::find_if(
				collada->m_GeometryLibrary.m_Geometries.begin(),
				collada->m_GeometryLibrary.m_Geometries.cend(),
				[&geometry_url_str, &position](const std::unique_ptr<DAE_Collada::Geometry> &itemgeometry) {
				position = geometry_url_str.find(itemgeometry->m_ID);
				if (position != geometry_url_str.npos)
				{
					return true;
				}
				else
				{
					return false;
				}
			});

			if (*itgeometry != nullptr)
			{
				CreateMesh3D(collada, itgeometry->get());
			}
		}
		else
		{

		}

		vector<std::unique_ptr<DAE_Collada::Node>>::const_iterator iterator =node->m_nodes.begin();
		for (; iterator != node->m_nodes.end(); iterator++)
		{
			std::shared_ptr<Entity3D> tempEntity3D(new Entity3D);
			tempEntity3D->CreateEntity3D(m_pD3dDevice, m_pCommandList, collada, iterator->get());			
			m_entity3d_list.push_back(tempEntity3D);
		}
	}





	void Entity3D::UpdateVertexAndIndexSubresources()
	{
		if (m_iEntity3DNumber > 0)			
		{
			const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);

			// 在 GPU 的默认堆中创建顶点缓冲区资源并使用上载堆将顶点数据复制到其中。
			// 在 GPU 使用完之前，不得释放上载资源。
			CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
				&defaultHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&vertexBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&this->m_vertexBuffer)));

			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&vertexBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_pVertexBufferUpload)));

			NAME_D3D12_OBJECT(this->m_vertexBuffer);


			// 将顶点缓冲区上载到 GPU。
			{
				D3D12_SUBRESOURCE_DATA vertexData = {};
				vertexData.pData = reinterpret_cast<BYTE*>(VertexArray.data());
				vertexData.RowPitch = vertexBufferSize;
				vertexData.SlicePitch = vertexData.RowPitch;

				UINT64 reslut = UpdateSubresources(m_pCommandList.Get(), m_vertexBuffer.Get(), m_pVertexBufferUpload.Get(), 0, 0, 1, &vertexData);
				CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
					CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
				m_pCommandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
			}

		}//---------------if (m_iEntity3DNumber > 0)	end ---------
		

		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateVertexAndIndexSubresources();
		}


	}



	void Entity3D::UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager)
	{
		if (m_iEntity3DNumber > 0)
		{
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer((DX::c_frameCount)  * c_AlignedConstantBufferSize);
			DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&constantBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_constantBuffer)));

			NAME_D3D12_OBJECT(m_constantBuffer);

			// 创建常量缓冲区视图以访问上载缓冲区。
			D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
			m_iStartOffset = cbvHeapManager->GetHeapCurrentOffset();
			m_cbvDescriptorSize = cbvHeapManager->GetCbvDescriptorSize();
			for (UINT n = 0; n < DX::c_frameCount*m_iEntity3DNumber; n++)
			{
				cbvHeapManager->CreateConstantBufferView(cbvGpuAddress, c_AlignedConstantBufferSize);
			}

			// 映射常量缓冲区。
			CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
			DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
			//DX::ThrowIfFailed(m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantBuffer)));

			ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount*m_iEntity3DNumber * c_AlignedConstantBufferSize);
			// 应用关闭之前，我们不会对此取消映射。在资源生命周期内使对象保持映射状态是可行的。		
		}//--------------if (m_iEntity3DNumber > 0)---------------end


		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpLoadVextureConstantBuffer(cbvHeapManager);
		}
	}

	void Entity3D::CreateBufferView()
	{
		if (m_iEntity3DNumber > 0)
		{
			const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);
			const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

			// 创建顶点/索引缓冲区视图。
			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColorForPlane);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;

			//m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			//m_indexBufferView.SizeInBytes = indexBufferSize;
			//m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		}

		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->CreateBufferView();
		}
	}
	void Entity3D::UpdateAndDraw(ConstantBufferViewHeapManager *cbvHeapManager, int currentFrameIndex, DirectX::XMMATRIX curMatrix, TextureManager *textureManager, UINT rootParameterIndex)
	{
		DirectX::XMMATRIX mulMatrix = DirectX::XMMatrixMultiply(m_matrix, curMatrix);

		if (m_iEntity3DNumber > 0)
		{
			
			int sphereCurrentFrameIndex =  currentFrameIndex;//m_iStartOffset
																						   //Update(sphereCurrentFrameIndex, curMatrix);			
			Update(sphereCurrentFrameIndex, mulMatrix);
			//	int 
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), sphereCurrentFrameIndex + m_iStartOffset, cbvHeapManager->GetCbvDescriptorSize());
			m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

			//draw
			std::wstring	cctv = fileStr + m_imageStr;
			textureManager->BindTexture(cbvHeapManager, cctv, 1, m_pCommandList);
			DrawEntity3D();
		}

		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->UpdateAndDraw(cbvHeapManager, currentFrameIndex, mulMatrix, textureManager, rootParameterIndex);
		}
	}

	void Entity3D::UpdateEntity3DByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix,  float angle)// float radians, DirectX::XMFLOAT3 offsetPos
	{

		DirectX::XMMATRIX mulMatrix= DirectX::XMMatrixMultiply(m_matrix, curMatrix);
		
		if (m_iEntity3DNumber > 0)
		{	
			assert(sphereIndex >= 0);
			assert(sphereIndex < m_iEntity3DNumber);
			int sphereCurrentFrameIndex = DX::c_frameCount*sphereIndex + currentFrameIndex;//m_iStartOffset
			//Update(sphereCurrentFrameIndex, curMatrix);			
			Update(sphereCurrentFrameIndex, mulMatrix);
			//	int 
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), sphereCurrentFrameIndex + m_iStartOffset, cbvHeapManager->GetCbvDescriptorSize());
			m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);			
		}

	}

	void Entity3D::DrawEntity3D()
	{
		if (m_iEntity3DNumber > 0)
		{	
			m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
			m_pCommandList->DrawInstanced(m_iIndices, 1, 0, 0);

			//m_pCommandList->IASetIndexBuffer(&m_indexBufferView);
			//m_pCommandList->DrawIndexedInstanced(m_iIndices, 1, 0, 0, 0);
		}

		
	}

	void Entity3D::DrawEntity3D(TextureManager *textureManager, ConstantBufferViewHeapManager *cbvHeapManager, UINT rootParameterIndex)
	{
		if (m_iEntity3DNumber > 0)
		{
			std::wstring	cctv = fileStr + m_imageStr;
			textureManager->BindTexture(cbvHeapManager, cctv, 1, m_pCommandList);
			DrawEntity3D();
		}
	}

	void Entity3D::RemoveUselessContent()
	{
		if (m_pVertexBufferUpload!=nullptr)
		{
			m_pVertexBufferUpload.Reset();
		}
		
		//m_pIndexBufferUpload.Reset();

		for (std::list<std::shared_ptr<Entity3D>>::iterator it = m_entity3d_list.begin(); it != m_entity3d_list.end(); ++it)
		{
			it->get()->RemoveUselessContent();
		}
	}

	void Entity3D::SetOriginalMatrix(std::vector<float> matrix)
	{		
		m_matrix.r[0] = XMVectorSet(matrix[0], matrix[4], matrix[8], matrix[12]);
		m_matrix.r[1] = XMVectorSet(matrix[1], matrix[5], matrix[9] , matrix[13]);
		m_matrix.r[2] = XMVectorSet(matrix[2], matrix[6] , matrix[10] , matrix[14]);
		m_matrix.r[3] = XMVectorSet(matrix[3], matrix[7], matrix[11], matrix[15] );
	}
	//-----------------------
}
