#include "pch.h"
#include "CustomSphere.h"
#include "..\Common\DirectXHelper.h"

#include <tchar.h> 

#include "G:\Mybooks\UWPSamplesDirectX12\DirectXTex-master\DirectXTex\DirectXTex.h"


using namespace DirectX;


namespace DirectX12FbxBase
{
	CustomSphere::CustomSphere()
	{
		
	}


	void CustomSphere::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber=1)
	{	
		assert(sphereNumber > 0);		
		m_iSphereNumber = sphereNumber;
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		ClearConstantBuffer();
		
	}

	void CustomSphere::CreateSphereVector()
	{


		int vertsPerRow = m_iSlice + 1;
		int nRows = m_iStack - 1;

		int nVerts = vertsPerRow * nRows + 2;
		//int nIndices = (nRows - 1)*slice * 6 + slice * 6;
		m_iIndices = (nRows - 1)*m_iSlice * 6 + m_iSlice * 6;
		VertexArray.clear();
		indexArray.clear();

		VertexArray.resize(nVerts);
		indexArray.resize(m_iIndices);

		for (int i = 1; i <= nRows; ++i)
		{
			float phy = XM_PI * i / m_iStack;
			float tmpRadius = m_fRadius * sin(phy);
			for (int j = 0; j < vertsPerRow; ++j)
			{
				float theta = XM_2PI * j / m_iSlice;
				UINT index = (i - 1)*vertsPerRow + j;

				float x = tmpRadius*cos(theta);
				float y = m_fRadius*cos(phy);
				float z = tmpRadius*sin(theta);

				//位置坐标
				VertexArray[index].pos = XMFLOAT3(x, y, z);
				VertexArray[index].color = XMFLOAT3(0.3f, 0.3f, 0.3f);// XMFLOAT3(1.0f, 0.0f, 0.0f);
																	  //法线
				XMVECTOR N = XMVectorSet(x, y, z, 0.f);
				XMStoreFloat3(&VertexArray[index].normal, XMVector3Normalize(N));
				//XMStoreFloat3(&mesh.vertices[index].normal, XMVector3Normalize(N));
				//切线
				XMVECTOR T = XMVectorSet(-sin(theta), 0.f, cos(theta), 0.f);
				XMStoreFloat3(&VertexArray[index].tangent, XMVector3Normalize(T));
				//纹理坐标
				VertexArray[index].uv = XMFLOAT2(j*1.f / m_iSlice, i*1.f / m_iStack);
			}
		}

		int size = vertsPerRow * nRows;
		//添加顶部和底部两个顶点信息
		VertexArray[size].pos = XMFLOAT3(0.f, m_fRadius, 0.f);
		VertexArray[size].color = XMFLOAT3(0.3f, 0.3f, 0.3f); //XMFLOAT3(0.0f, 0.0f, 1.0f);
		VertexArray[size].normal = XMFLOAT3(0.f, 1.f, 0.f);
		VertexArray[size].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[size].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[size + 1].pos = XMFLOAT3(0.f, -m_fRadius, 0.f);
		VertexArray[size + 1].color = XMFLOAT3(0.3f, 0.3f, 0.3f);// XMFLOAT3(0.0f, 0.0f, 1.0f);
		VertexArray[size + 1].normal = XMFLOAT3(0.f, -1.f, 0.f);
		VertexArray[size + 1].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[size + 1].uv = XMFLOAT2(0.f, 1.f);
		//sizeof(VertexArray) / sizeof(VertexArray[0])	
		UINT tmp(0);
		int start1 = 0;
		int start2 = nVerts - vertsPerRow - 2;
		int top = size;
		int bottom = size + 1;
		for (int i = 0; i < m_iSlice; ++i)
		{
			indexArray[tmp] = top;
			indexArray[tmp + 1] = start1 + i + 1;
			indexArray[tmp + 2] = start1 + i;

			tmp += 3;
		}

		for (int i = 0; i < m_iSlice; ++i)
		{
			indexArray[tmp] = bottom;
			indexArray[tmp + 1] = start2 + i;
			indexArray[tmp + 2] = start2 + i + 1;

			tmp += 3;
		}

		for (int i = 0; i < nRows - 1; ++i)
		{
			for (int j = 0; j < m_iSlice; ++j)
			{
				indexArray[tmp] = i * vertsPerRow + j;
				indexArray[tmp + 1] = (i + 1) * vertsPerRow + j + 1;
				indexArray[tmp + 2] = (i + 1) * vertsPerRow + j;
				indexArray[tmp + 3] = i * vertsPerRow + j;
				indexArray[tmp + 4] = i * vertsPerRow + j + 1;
				indexArray[tmp + 5] = (i + 1) * vertsPerRow + j + 1;

				tmp += 6;
			}
		}

	}

	void CustomSphere::UpdateVertexAndIndexSubresources()	
	{		
		const UINT vertexBufferSize =(UINT)VertexArray.size() * sizeof(VertexArray[0]);
		const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

		TCHAR  sOut[512];
		_stprintf_s(sOut, L"圆球——kao————1:  vertexBufferSize: %d; indexBufferSize: %d;\n", vertexBufferSize, indexBufferSize);
		//_stprintf_s("%d", _T(UINT));
		OutputDebugString(sOut);

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



		//const UINT indexBufferSize = indexArray.size() * sizeof(indexArray[0]);
		// 在 GPU 的默认堆中创建索引缓冲区资源并使用上载堆将索引数据复制到其中。
		// 在 GPU 使用完之前，不得释放上载资源。
		//Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

		CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&this->m_indexBuffer)));


		DX::ThrowIfFailed(m_pD3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pIndexBufferUpload)));

		NAME_D3D12_OBJECT(this->m_indexBuffer);


		// 将索引缓冲区上载到 GPU。
		{
			D3D12_SUBRESOURCE_DATA indexData = {};
			//indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
			indexData.pData = reinterpret_cast<BYTE*>(indexArray.data());
			indexData.RowPitch = indexBufferSize;
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresources(m_pCommandList.Get(), m_indexBuffer.Get(), m_pIndexBufferUpload.Get(), 0, 0, 1, &indexData);

			CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);		
			m_pCommandList->ResourceBarrier(1, &indexBufferResourceBarrier);
		}
	}

	void CustomSphere::UpLoadVextureConstantBuffer( ConstantBufferViewHeapManager *cbvHeapManager)
	{		
		/*assert(startOffset >= 0);*/
		//m_pCbvHeapManager = cbvHeapManager;
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer((DX::c_frameCount )  * c_AlignedConstantBufferSize);
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
		for (UINT n = 0; n < DX::c_frameCount*m_iSphereNumber; n++)
		{			
			cbvHeapManager->CreateConstantBufferView(cbvGpuAddress, c_AlignedConstantBufferSize);			
		}

		// 映射常量缓冲区。
		CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount*m_iSphereNumber * c_AlignedConstantBufferSize);
		// 应用关闭之前，我们不会对此取消映射。在资源生命周期内使对象保持映射状态是可行的。		
	}

	void CustomSphere::CreateBufferView()
	{
		const UINT vertexBufferSize =(UINT) VertexArray.size() * sizeof(VertexArray[0]);
		const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

		// 创建顶点/索引缓冲区视图。
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);	
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();	
		m_indexBufferView.SizeInBytes = indexBufferSize;
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	
	void CustomSphere::UpdateSphereByIndex(ConstantBufferViewHeapManager *cbvHeapManager,int sphereIndex,int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos)
	{
		assert(sphereIndex >= 0);
		assert(sphereIndex <m_iSphereNumber);
		int sphereCurrentFrameIndex =   DX::c_frameCount*sphereIndex + currentFrameIndex;
		Update(sphereCurrentFrameIndex,  radians, offsetPos);
		
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), m_iStartOffset+sphereCurrentFrameIndex, cbvHeapManager->GetCbvDescriptorSize());
		m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	}
	
	void CustomSphere::Update(int currentFrameIndex, float radians, DirectX::XMFLOAT3 offsetPos)
	{
	
		XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(offsetPos.x, offsetPos.y, offsetPos.z)*XMMatrixRotationY(m_angle)));
		m_constantBufferData.lightDirction = DirectX::XMFLOAT3(m_fLightX, m_fLightY, m_fLightZ);
		m_constantBufferData.cameraPosition= DirectX::XMFLOAT3(m_fCameraX, m_fCameraY, m_fCameraZ);
		// 更新常量缓冲区资源。
		UINT8* destination = m_mappedConstantBuffer + (currentFrameIndex * c_AlignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));
	}

	void CustomSphere::DrawSphere()
	{
		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_pCommandList->IASetIndexBuffer(&m_indexBufferView);
		m_pCommandList->DrawIndexedInstanced(m_iIndices, 1, 0, 0, 0);
	}


	void CustomSphere::RemoveUselessContent()
	{
		m_pVertexBufferUpload.Reset();
		m_pIndexBufferUpload.Reset();
		//m_pTextureUpload.Reset();
		//vertexBufferUpload.re
	}

	void CustomSphere::ClearConstantBuffer()
	{
		if (m_constantBuffer!=nullptr)
		{
			m_constantBuffer->Unmap(0, nullptr);
		}		
		m_mappedConstantBuffer = nullptr;
	}

	void CustomSphere::UpdateElapsedSeconds(float elapsedTime)
	{
		if (m_isRotate)
		{
			// 少量旋转立方体。
			m_angle += elapsedTime;

		}
	}

	void CustomSphere::SetRotateEnable()
	{
		if (m_isRotate)
		{
			m_isRotate = false;
		}
		else
		{
			m_isRotate = true;
		}
		
	}


}