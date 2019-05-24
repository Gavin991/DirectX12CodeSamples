#include "pch.h"
#include "CustomCube.h"
#include "..\Common\DirectXHelper.h"
#include <tchar.h> 
#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"

using namespace DirectX;

namespace DirectX12XAMLXAudio2
{
	CustomCube::CustomCube()
	{
	}

	void CustomCube::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, int sphereNumber = 1)
	{
		assert(sphereNumber > 0);
		m_iSphereNumber = sphereNumber;
		m_pD3dDevice = device;
		m_pCommandList = commandList;
		ClearConstantBuffer();
	}

	void CustomCube::ClearConstantBuffer()
	{
		if (m_constantBuffer != nullptr)
		{
			m_constantBuffer->Unmap(0, nullptr);
		}
		m_mappedConstantBuffer = nullptr;

	}

	void CustomCube::CreateCubeVector()
	{		
		VertexArray.clear();
		indexArray.clear();

		//һ��24������(ÿ��4��)
		VertexArray.resize(24);
		//һ��36������(ÿ��6��)
		indexArray.resize(36);

		m_iIndices = 36;

		float halfW = m_fWidth * 0.5f;
		float halfH = m_fHeight * 0.5f;
		float halfD = m_fdepth * 0.5f;

		//�۾�����z��������
		//��������
		//ǰ��
		VertexArray[0].pos = XMFLOAT3(-halfW, -halfH, -halfD);
		VertexArray[0].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[0].normal = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[0].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[0].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[1].pos = XMFLOAT3(-halfW, halfH, -halfD);
		VertexArray[1].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[1].normal = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[1].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[1].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[2].pos = XMFLOAT3(halfW, halfH, -halfD);
		VertexArray[2].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[2].normal = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[2].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[2].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[3].pos = XMFLOAT3(halfW, -halfH, -halfD);
		VertexArray[3].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[3].normal = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[3].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[3].uv = XMFLOAT2(1.f, 1.f);

		//�����
		VertexArray[4].pos = XMFLOAT3(-halfW, -halfH, halfD);
		VertexArray[4].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[4].normal = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[4].tangent = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[4].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[5].pos = XMFLOAT3(-halfW, halfH, halfD);
		VertexArray[5].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[5].normal = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[5].tangent = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[5].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[6].pos = XMFLOAT3(-halfW, halfH, -halfD);
		VertexArray[6].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[6].normal = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[6].tangent = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[6].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[7].pos = XMFLOAT3(-halfW, -halfH, -halfD);
		VertexArray[7].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[7].normal = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[7].tangent = XMFLOAT3(0.f, 0.f, -1.f);
		VertexArray[7].uv = XMFLOAT2(1.f, 1.f);

		//����
		VertexArray[8].pos = XMFLOAT3(halfW, -halfH, halfD);
		VertexArray[8].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[8].normal = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[8].tangent = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[8].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[9].pos = XMFLOAT3(halfW, halfH, halfD);
		VertexArray[9].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[9].normal = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[9].tangent = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[9].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[10].pos = XMFLOAT3(-halfW, halfH, halfD);
		VertexArray[10].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[10].normal = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[10].tangent = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[10].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[11].pos = XMFLOAT3(-halfW, -halfH, halfD);
		VertexArray[11].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[11].normal = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[11].tangent = XMFLOAT3(-1.f, 0.f, 0.f);
		VertexArray[11].uv = XMFLOAT2(1.f, 1.f);

		//�Ҳ���
		VertexArray[12].pos = XMFLOAT3(halfW, -halfH, -halfD);
		VertexArray[12].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[12].normal = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[12].tangent = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[12].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[13].pos = XMFLOAT3(halfW, halfH, -halfD);
		VertexArray[13].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[13].normal = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[13].tangent = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[13].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[14].pos = XMFLOAT3(halfW, halfH, halfD);
		VertexArray[14].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[14].normal = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[14].tangent = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[14].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[15].pos = XMFLOAT3(halfW, -halfH, halfD);
		VertexArray[15].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[15].normal = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[15].tangent = XMFLOAT3(0.f, 0.f, 1.f);
		VertexArray[15].uv = XMFLOAT2(1.f, 1.f);

		//����
		VertexArray[16].pos = XMFLOAT3(-halfW, halfH, -halfD);
		VertexArray[16].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[16].normal = XMFLOAT3(0.f, 1.f, 0.f);
		VertexArray[16].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[16].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[17].pos = XMFLOAT3(-halfW, halfH, halfD);
		VertexArray[17].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[17].normal = XMFLOAT3(0.f, 1.f, 0.f);
		VertexArray[17].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[17].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[18].pos = XMFLOAT3(halfW, halfH, halfD);
		VertexArray[18].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[18].normal = XMFLOAT3(0.f, 1.f, 0.f);
		VertexArray[18].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[18].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[19].pos = XMFLOAT3(halfW, halfH, -halfD);
		VertexArray[19].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[19].normal = XMFLOAT3(0.f, 1.f, 0.f);
		VertexArray[19].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[19].uv = XMFLOAT2(1.f, 1.f);
		//����
		VertexArray[20].pos = XMFLOAT3(-halfW, -halfH, halfD);
		VertexArray[20].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[20].normal = XMFLOAT3(0.f, -1.f, 0.f);
		VertexArray[20].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[20].uv = XMFLOAT2(0.f, 1.f);

		VertexArray[21].pos = XMFLOAT3(-halfW, -halfH, -halfD);
		VertexArray[21].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[21].normal = XMFLOAT3(0.f, -1.f, 0.f);
		VertexArray[21].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[21].uv = XMFLOAT2(0.f, 0.f);

		VertexArray[22].pos = XMFLOAT3(halfW, -halfH, -halfD);
		VertexArray[22].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[22].normal = XMFLOAT3(0.f, -1.f, 0.f);
		VertexArray[22].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[22].uv = XMFLOAT2(1.f, 0.f);

		VertexArray[23].pos = XMFLOAT3(halfW, -halfH, halfD);
		VertexArray[23].color = XMFLOAT3(0.3f, 0.3f, 0.3f);
		VertexArray[23].normal = XMFLOAT3(0.f, -1.f, 0.f);
		VertexArray[23].tangent = XMFLOAT3(1.f, 0.f, 0.f);
		VertexArray[23].uv = XMFLOAT2(1.f, 1.f);

		//��������
		indexArray[0] = 0;
		indexArray[1] = 2;
		indexArray[2] = 1;
		indexArray[3] = 0;
		indexArray[4] = 3;
		indexArray[5] = 2;

		indexArray[6] = 4;
		indexArray[7] = 6;
		indexArray[8] = 5;
		indexArray[9] = 4;
		indexArray[10] = 7;
		indexArray[11] = 6;

		indexArray[12] = 8;
		indexArray[13] = 10;
		indexArray[14] = 9;
		indexArray[15] = 8;
		indexArray[16] = 11;
		indexArray[17] = 10;

		indexArray[18] = 12;
		indexArray[19] = 14;
		indexArray[20] = 13;
		indexArray[21] = 12;
		indexArray[22] = 15;
		indexArray[23] = 14;

		indexArray[24] = 16;
		indexArray[25] = 18;
		indexArray[26] = 17;
		indexArray[27] = 16;
		indexArray[28] = 19;
		indexArray[29] = 18;

		indexArray[30] = 20;
		indexArray[31] = 22;
		indexArray[32] = 21;
		indexArray[33] = 20;
		indexArray[34] = 23;
		indexArray[35] = 22;
	}



	void CustomCube::UpdateVertexAndIndexSubresources()
	{
		const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);
		const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

		TCHAR  sOut[512];
		_stprintf_s(sOut, L"Բ�򡪡�kao��������1:  vertexBufferSize: %d; indexBufferSize: %d;\n", vertexBufferSize, indexBufferSize);
		//_stprintf_s("%d", _T(UINT));
		OutputDebugString(sOut);

		// �� GPU ��Ĭ�϶��д������㻺������Դ��ʹ�����ضѽ��������ݸ��Ƶ����С�
		// �� GPU ʹ����֮ǰ�������ͷ�������Դ��
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


		// �����㻺�������ص� GPU��
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
		// �� GPU ��Ĭ�϶��д���������������Դ��ʹ�����ضѽ��������ݸ��Ƶ����С�
		// �� GPU ʹ����֮ǰ�������ͷ�������Դ��
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


		// ���������������ص� GPU��
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


	void CustomCube::UpLoadVextureConstantBuffer(ConstantBufferViewHeapManager *cbvHeapManager)
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

		// ����������������ͼ�Է������ػ�������
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
		m_iStartOffset = cbvHeapManager->GetHeapCurrentOffset();
		m_cbvDescriptorSize = cbvHeapManager->GetCbvDescriptorSize();
		for (UINT n = 0; n < DX::c_frameCount*m_iSphereNumber; n++)
		{
			cbvHeapManager->CreateConstantBufferView(cbvGpuAddress, c_AlignedConstantBufferSize);			
		}

		// ӳ�䳣����������
		CD3DX12_RANGE readRange(0, 0);		// ���ǲ������ CPU �ϵĴ���Դ�н��ж�ȡ��
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		//DX::ThrowIfFailed(m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantBuffer)));

		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount*m_iSphereNumber * c_AlignedConstantBufferSize);
		// Ӧ�ùر�֮ǰ�����ǲ���Դ�ȡ��ӳ�䡣����Դ����������ʹ���󱣳�ӳ��״̬�ǿ��еġ�		
	}



	void CustomCube::CreateBufferView()
	{
		const UINT vertexBufferSize = (UINT)VertexArray.size() * sizeof(VertexArray[0]);
		const UINT indexBufferSize = (UINT)indexArray.size() * sizeof(indexArray[0]);

		// ��������/������������ͼ��
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = indexBufferSize;
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}



	void CustomCube::Update(int currentFrameIndex, DirectX::XMMATRIX curMatrix)// float radians, DirectX::XMFLOAT3 offsetPos
	{

		XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(curMatrix));//*XMMatrixRotationY(radians) XMMatrixTranslation(offsetPos.x, offsetPos.y, offsetPos.z)
	
		m_constantBufferData.lightDirction = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);

		// ���³�����������Դ��
		UINT8* destination = m_mappedConstantBuffer + (currentFrameIndex * c_AlignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

	}


	void CustomCube::UpdateCubeByIndex(ConstantBufferViewHeapManager *cbvHeapManager, int sphereIndex, int currentFrameIndex, DirectX::XMMATRIX curMatrix)// float radians, DirectX::XMFLOAT3 offsetPos
	{
		assert(sphereIndex >= 0);
		assert(sphereIndex < m_iSphereNumber);
		int sphereCurrentFrameIndex = DX::c_frameCount*sphereIndex + currentFrameIndex;//m_iStartOffset
		Update(sphereCurrentFrameIndex, curMatrix);

	//	int 
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(cbvHeapManager->GetGPUDescriptorHandleForHeapStart(), sphereCurrentFrameIndex+ m_iStartOffset, cbvHeapManager->GetCbvDescriptorSize());
		m_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	}

	void CustomCube::DrawCube(int currentFrameIndex)
	{
		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_pCommandList->IASetIndexBuffer(&m_indexBufferView);
		m_pCommandList->DrawIndexedInstanced(m_iIndices, 1, 0, 0, 0);
	}

	void CustomCube::RemoveUselessContent()
	{
		m_pVertexBufferUpload.Reset();
		m_pIndexBufferUpload.Reset();		
	}

}
