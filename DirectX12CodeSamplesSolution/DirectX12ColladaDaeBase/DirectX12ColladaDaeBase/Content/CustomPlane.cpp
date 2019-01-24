#include "pch.h"
#include "CustomPlane.h"
#include "..\Common\DirectXHelper.h"
#include <tchar.h> 

using namespace DirectX;

namespace DirectX12ColladaDaeBase
{
	CustomPlane::CustomPlane()
	{
	}

	void CustomPlane::Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix)// float radians, DirectX::XMFLOAT3 offsetPos
	{

		XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(curMatrix));

	

		// 更新常量缓冲区资源。
		UINT8* destination = m_mappedConstantBuffer + (currentFrameIndex * c_AlignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

	}

	void CustomPlane::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber = 1)
	{
		assert(sphereNumber > 0);
		m_iSphereNumber = sphereNumber;
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		ClearConstantBuffer();
	}

	void CustomPlane::ClearConstantBuffer()
	{
		if (m_constantBuffer != nullptr)
		{
			m_constantBuffer->Unmap(0, nullptr);
		}
		m_mappedConstantBuffer = nullptr;

	}

	void CustomPlane::CreatePlaneVector()
	{
		////每行顶点数、每列顶点数
		float halfWidth = 0.5f*width;
		float halfDepth = 0.5f*depth;

		float dx = width / (n - 1);
		float dz = depth / (m - 1);

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		uint32 vertexCount = m*n;//5*5=25
		uint32 faceCount = (m - 1)*(n - 1) * 2;//32

		VertexArray.clear();
		indexArray.clear();

		//一共24个顶点(每面4个)
		VertexArray.resize(25);
		//一共36个索引(每面6个)
		indexArray.resize(96);

		m_iIndices = 96;


		for (uint32 i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dz;
			for (uint32 j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				VertexArray[i*n + j].pos = XMFLOAT3(x, 0.0f, z);
				//meshData.Vertices[i*n + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				//meshData.Vertices[i*n + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);
				VertexArray[i*n + j].color = XMFLOAT3(1.0f, 1.0f, 1.0f);
				// Stretch texture over grid.
				VertexArray[i*n + j].uv.x = (float)j;// j*du;
				VertexArray[i*n + j].uv.y = (float)i;// i*dv;
			}
		}


		uint32 k = 0;
		for (uint32 i = 0; i < m - 1; ++i)
		{
			for (uint32 j = 0; j < n - 1; ++j)
			{
				indexArray[k] = i*n + j;
				indexArray[k + 2] = i*n + j + 1;
				indexArray[k + 1] = (i + 1)*n + j;
				//indexArray[k + 1] = i*n + j + 1;
				//indexArray[k + 2] = (i + 1)*n + j;

				indexArray[k + 3] = (i + 1)*n + j;
				indexArray[k + 5] = i*n + j + 1;
				indexArray[k + 4] = (i + 1)*n + j + 1;
				//indexArray[k + 4] = i*n + j + 1;
				//indexArray[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}	
	
	
	
	}



	void CustomPlane::UpdateVertexAndIndexSubresources()
	{
		const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);
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



	void CustomPlane::UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager)
	{
		//assert(startOffset >= 0);
		//m_pCbvHeapManager = cbvHeapManager;
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
		for (UINT n = 0; n < DX::c_frameCount*m_iSphereNumber; n++)
		{
			cbvHeapManager->CreateConstantBufferView(cbvGpuAddress, c_AlignedConstantBufferSize);
		}

		// 映射常量缓冲区。
		CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		//DX::ThrowIfFailed(m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantBuffer)));

		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount*m_iSphereNumber * c_AlignedConstantBufferSize);
		// 应用关闭之前，我们不会对此取消映射。在资源生命周期内使对象保持映射状态是可行的。		
	}

	void CustomPlane::CreateBufferView()
	{
		const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);
		const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

		// 创建顶点/索引缓冲区视图。
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColorForPlane);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = indexBufferSize;
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}


	void CustomPlane::UpdatePlaneByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix)// float radians, DirectX::XMFLOAT3 offsetPos
	{
		assert(sphereIndex >= 0);
		assert(sphereIndex < m_iSphereNumber);
		int sphereCurrentFrameIndex = DX::c_frameCount*sphereIndex + currentFrameIndex;//m_iStartOffset
		Update(sphereCurrentFrameIndex, curMatrix);

		//	int 
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), sphereCurrentFrameIndex + m_iStartOffset, cbvHeapManager->GetCbvDescriptorSize());
		m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	}

	void CustomPlane::DrawPlane(int currentFrameIndex)
	{
		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_pCommandList->IASetIndexBuffer(&m_indexBufferView);
		m_pCommandList->DrawIndexedInstanced(m_iIndices, 1, 0, 0, 0);
	}

	void CustomPlane::RemoveUselessContent()
	{
		m_pVertexBufferUpload.Reset();
		m_pIndexBufferUpload.Reset();
	}
	//-----------------------
}