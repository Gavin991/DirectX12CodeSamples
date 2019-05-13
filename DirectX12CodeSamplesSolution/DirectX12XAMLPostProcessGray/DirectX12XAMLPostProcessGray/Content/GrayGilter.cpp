#include "pch.h"
#include "GrayGilter.h"
#include "..\Common\DirectXHelper.h"

namespace DirectX12XAMLPostProcessGray
{
	GrayGilter::GrayGilter()
	{
	}


	GrayGilter::GrayGilter(ID3D12Device* device,
		UINT width, UINT height,
		DXGI_FORMAT format)
	{
		md3dDevice = device;

		mWidth = width;
		mHeight = height;
		mFormat = format;

		BuildResources();
	}

	ID3D12Resource* GrayGilter::Output()
	{
		return mGrayMap.Get();
	}


	void GrayGilter::BuildDescriptorsForGray(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize)
	{
		// 保存描述符的引用。 
		mGrayCpuSrv = hCpuDescriptor;
		mGrayCpuUav = hCpuDescriptor.Offset(1, descriptorSize);

		mGrayGpuSrv = hGpuDescriptor;
		mGrayGpuUav = hGpuDescriptor.Offset(1, descriptorSize);

		BuildSrvAndUav();
	}

	void GrayGilter::OnResize(UINT newWidth, UINT newHeight)
	{
		if ((mWidth != newWidth) || (mHeight != newHeight))
		{
			mWidth = newWidth;
			mHeight = newHeight;

			BuildResources();

			//窗口改变，要创建新的描述符
			BuildSrvAndUav();
		}
	}

	void GrayGilter::BuildResources()
	{		
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = mWidth;
		texDesc.Height = mHeight;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = mFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		DX::ThrowIfFailed(md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&mGrayMap)));
	}

	void GrayGilter::BuildSrvAndUav()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = mFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

		uavDesc.Format = mFormat;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		md3dDevice->CreateShaderResourceView(mGrayMap.Get(), &srvDesc, mGrayCpuSrv);
		md3dDevice->CreateUnorderedAccessView(mGrayMap.Get(), nullptr, &uavDesc, mGrayCpuUav);

	}

	void GrayGilter::GrayBehavior(ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* grayPSO,
		ID3D12Resource* input)
	{
		
		cmdList->SetComputeRootSignature(rootSig);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGrayMap.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		// Copy the input (back-buffer in this example) to BlurMap0.
		cmdList->CopyResource(mGrayMap.Get(), input);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGrayMap.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));


		cmdList->SetPipelineState(grayPSO);

		cmdList->SetComputeRootDescriptorTable(0, mGrayGpuSrv);
		cmdList->SetComputeRootDescriptorTable(1, mGrayGpuUav);
	
		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(mWidth / 32.0f);
		UINT numGroupsY = (UINT)ceilf(mHeight / 32.0f);
		cmdList->Dispatch(numGroupsX, numGroupsY, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mGrayMap.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));




	}

	//-----------namespace DirectX12XAMLPostProcess-----------------------end
}