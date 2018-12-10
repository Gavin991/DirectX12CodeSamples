#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include <ppltasks.h>
#include <synchapi.h>

#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"

using namespace DirectX12UWPLight;

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Storage;

// 应用程序状态映射中的索引。
Platform::String^ AngleKey = "Angle";
Platform::String^ TrackingKey = "Tracking";

void CreateSphereVector(float radius, int slice, int stack, std::vector<VertexPositionColor> &VertexArray, std::vector<unsigned short> &indexArray);
int nIndices;

// 从文件中加载顶点和像素着色器，然后实例化立方体几何图形。
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_radiansPerSecond(XM_PIDIV4),	// 每秒旋转 45 度
	m_angle(0),
	m_tracking(false),
	m_mappedConstantBuffer(nullptr),
	m_deviceResources(deviceResources)
{
	LoadState();
	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

Sample3DSceneRenderer::~Sample3DSceneRenderer()
{
	m_constantBuffer->Unmap(0, nullptr);
	m_mappedConstantBuffer = nullptr;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	auto d3dDevice = m_deviceResources->GetD3DDevice();
	
	//// 创建具有单个常量缓冲区槽的根签名。
	//{
	//	CD3DX12_DESCRIPTOR_RANGE range;
	//	CD3DX12_ROOT_PARAMETER parameter;

	//	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//	parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

	//	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
	//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // 只有输入汇编程序阶段才需要访问常量缓冲区。
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
	//		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	//	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	//	descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

	//	ComPtr<ID3DBlob> pSignature;
	//	ComPtr<ID3DBlob> pError;
	//	DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
	//	DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
 //       NAME_D3D12_OBJECT(m_rootSignature);
	//}

	// 创建具有单个常量缓冲区槽的根签名。
	{

		// create sampler 0 in register 0 in pixel shader
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 16;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_DESCRIPTOR_RANGE range1;
		range1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range1.NumDescriptors = 1;
		range1.BaseShaderRegister = 0;
		range1.RegisterSpace = 0;
		range1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter1;
		parameter1.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter1.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameter1.DescriptorTable.NumDescriptorRanges = 1;
		parameter1.DescriptorTable.pDescriptorRanges = &range1;

		D3D12_DESCRIPTOR_RANGE range2;
		range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range2.NumDescriptors = 1;
		range2.BaseShaderRegister = 0;
		range2.RegisterSpace = 0;
		range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter2;
		parameter2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter2.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameter2.DescriptorTable.NumDescriptorRanges = 1;
		parameter2.DescriptorTable.pDescriptorRanges = &range2;


		D3D12_DESCRIPTOR_RANGE range3;
		range3.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range3.NumDescriptors = 1;
		range3.BaseShaderRegister = 0; // first pixel cbv buffer so it can stay zero
		range3.RegisterSpace = 0;
		range3.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter3;
		parameter3.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter3.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameter3.DescriptorTable.NumDescriptorRanges = 1;
		parameter3.DescriptorTable.pDescriptorRanges = &range3;


		D3D12_ROOT_PARAMETER parameters[3];
		parameters[0] = parameter1;
		parameters[1] = parameter2;
		parameters[2] = parameter3;


		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // Only the input assembler stage needs access to the constant buffer.
																									  //| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
																									  //D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
																									  //D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
																									  //D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(_countof(parameters), &parameters[0], 1, &sampler, rootSignatureFlags);

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	}


	// 通过异步方式加载着色器。
	auto createVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso").then([this](std::vector<byte>& fileData) {
		m_vertexShader = fileData;
	});

	auto createPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso").then([this](std::vector<byte>& fileData) {
		m_pixelShader = fileData;
	});

	// 加载着色器之后创建管道状态。
	auto createPipelineStateTask = (createPSTask && createVSTask).then([this]() {

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { inputLayout, _countof(inputLayout) };
		state.pRootSignature = m_rootSignature.Get();
        state.VS = CD3DX12_SHADER_BYTECODE(&m_vertexShader[0], m_vertexShader.size());
        state.PS = CD3DX12_SHADER_BYTECODE(&m_pixelShader[0], m_pixelShader.size());
		state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		state.NumRenderTargets = 1;
		state.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
		state.DSVFormat = m_deviceResources->GetDepthBufferFormat();
		state.SampleDesc.Count = 1;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&m_pipelineState)));

		// 创建管道状态之后可以删除着色器数据。
		m_vertexShader.clear();
		m_pixelShader.clear();
	});

	// 创建立方体几何图形资源并上载到 GPU。
	auto createAssetsTask = createPipelineStateTask.then([this]() {
		auto d3dDevice = m_deviceResources->GetD3DDevice();

		// 创建命令列表。
		DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
        NAME_D3D12_OBJECT(m_commandList);

		//// 立方体顶点。每个顶点都有一个位置和一个颜色。
		//VertexPositionColor cubeVertices[] =
		//{
		//	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
		//	{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		//	{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		//	{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
		//	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		//	{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
		//	{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		//	{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		//};

		//const UINT vertexBufferSize = sizeof(cubeVertices);


		float radius = 0.5;
		int slice = 100;
		int stack = 100;

		int vertsPerRow = slice + 1;//6
		int nRows = stack - 1;//4
		const int nVerts = vertsPerRow * nRows + 2;
		nIndices = (nRows - 1)*slice * 6 + slice * 6;//3*5*6+5*6;

		std::vector<VertexPositionColor> VertexCollection;
		std::vector<unsigned short> IndexCollection;
		CreateSphereVector(radius, slice, stack, VertexCollection, IndexCollection);

		const UINT vertexBufferSize = VertexCollection.size() * sizeof(VertexCollection[0]);


		// 在 GPU 的默认堆中创建顶点缓冲区资源并使用上载堆将顶点数据复制到其中。
		// 在 GPU 使用完之前，不得释放上载资源。
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));

        NAME_D3D12_OBJECT(m_vertexBuffer);

		// 将顶点缓冲区上载到 GPU。
		{
			D3D12_SUBRESOURCE_DATA vertexData = {};
			//vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
			vertexData.pData = reinterpret_cast<BYTE*>(VertexCollection.data());
			vertexData.RowPitch = vertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

			CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
		}

		//// 加载网格索引。每三个索引表示要在屏幕上呈现的三角形。
		//// 例如: 0,2,1 表示顶点缓冲区中的索引为 0、2 和 1 的顶点构成
		//// 此网格的第一个三角形。
		//unsigned short cubeIndices[] =
		//{
		//	0, 2, 1, // -x
		//	1, 2, 3,

		//	4, 5, 6, // +x
		//	5, 7, 6,

		//	0, 1, 5, // -y
		//	0, 5, 4,

		//	2, 6, 7, // +y
		//	2, 7, 3,

		//	0, 4, 6, // -z
		//	0, 6, 2,

		//	1, 3, 7, // +z
		//	1, 7, 5,
		//};

		//const UINT indexBufferSize = sizeof(cubeIndices);

		const UINT indexBufferSize = IndexCollection.size() * sizeof(IndexCollection[0]);
		// 在 GPU 的默认堆中创建索引缓冲区资源并使用上载堆将索引数据复制到其中。
		// 在 GPU 使用完之前，不得释放上载资源。
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

		CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)));

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBufferUpload)));

		NAME_D3D12_OBJECT(m_indexBuffer);

		// 将索引缓冲区上载到 GPU。
		{
			D3D12_SUBRESOURCE_DATA indexData = {};
			//indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
			indexData.pData = reinterpret_cast<BYTE*>(IndexCollection.data());
			indexData.RowPitch = indexBufferSize;
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

			CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
		}

		// 为常量缓冲区创建描述符堆。
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			//heapDesc.NumDescriptors = DX::c_frameCount+1;//因为了多来一个纹理采样器
			heapDesc.NumDescriptors = DX::c_frameCount // Vertex constant buffers per frame
				+ 1 // texture
				+ 1 // Pixel constant buffer
				;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// 此标志指示此描述符堆可以绑定到管道，并且其中包含的描述符可以由根表引用。
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

            NAME_D3D12_OBJECT(m_cbvHeap);
		}

		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(DX::c_frameCount * c_alignedConstantBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

        NAME_D3D12_OBJECT(m_constantBuffer);

		// 创建常量缓冲区视图以访问上载缓冲区。
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		m_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (int n = 0; n < DX::c_frameCount; n++)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = c_alignedConstantBufferSize;
			d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

			cbvGpuAddress += desc.SizeInBytes;
			cbvCpuHandle.Offset(m_cbvDescriptorSize);
		}

		// 映射常量缓冲区。
		CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount * c_alignedConstantBufferSize);
		// 应用关闭之前，我们不会对此取消映射。在资源生命周期内使对象保持映射状态是可行的。



#pragma region 加载纹理部分
		LPCWSTR szFile = L"Assets\\pic.jpg";
		DWORD flags = CP_FLAGS::CP_FLAGS_NONE;
		ScratchImage image;
		DirectX::LoadFromWICFile(szFile, flags, nullptr, image);

		auto metadata1 = image.GetMetadata();
		auto texture = image.GetPixels();

		CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::
			Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, metadata1.width, metadata1.height, 1, 1);

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_texture)));

		ComPtr<ID3D12Resource> uploadResources;
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1) + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadResources)));

		m_texture->SetName(L"Shader Resource Texture");
		uploadResources->SetName(L"Shader Upload Resource");

		// Upload the Shader Resource to the GPU.
		{
			// Copy data to the intermediate upload heap and then schedule a copy
			// from the upload heap to the texture.
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = texture;//reinterpret_cast<UINT8*>(texture.data());
			textureData.RowPitch = image.GetImages()->rowPitch;
			textureData.SlicePitch = textureData.RowPitch * metadata1.height;

			UpdateSubresources(m_commandList.Get(), m_texture.Get(), uploadResources.Get(), 0, 0, 1, &textureData);

			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

			d3dDevice->CreateShaderResourceView(m_texture.Get(), &srvDesc, cbvCpuHandle);
			cbvCpuHandle.Offset(m_cbvDescriptorSize);
		}
#pragma endregion


#pragma region Pixel Shader Constant Buffer
		{
			DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(c_alignedConstantPSBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_constantPSBuffer)));

			m_constantPSBuffer->SetName(L"Pixel Shader Constant Buffer");

			// Create constant buffer views to access the upload buffer.
			D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantPSBuffer->GetGPUVirtualAddress();

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = c_alignedConstantPSBufferSize;
			d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

			cbvCpuHandle.Offset(m_cbvDescriptorSize);

			// Map the constant buffers.
			DX::ThrowIfFailed(m_constantPSBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantPSBuffer)));
			ZeroMemory(m_mappedConstantPSBuffer, c_alignedConstantPSBufferSize);
			// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.
		}
#pragma endregion


		// 关闭命令列表并执行它，以开始将顶点/索引缓冲区复制到 GPU 的默认堆中。
		DX::ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// 创建顶点/索引缓冲区视图。
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
		//m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		//m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
		m_indexBufferView.SizeInBytes = indexBufferSize;
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

		// 等待命令列表完成执行；顶点/索引缓冲区需要在上载资源超出范围之前上载到 GPU。
		m_deviceResources->WaitForGpu();
	});

	createAssetsTask.then([this]() {
		m_loadingComplete = true;
	});
}

// 当窗口的大小改变时初始化视图参数。
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
	m_scissorRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height)};

	// 这是一个简单的更改示例，当应用在纵向视图或对齐视图中时，可以进行此更改
	//。
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// 请注意，OrientationTransform3D 矩阵在此处是后乘的，
	// 以正确确定场景的方向，使之与显示方向匹配。
	// 对于交换链的目标位图进行的任何绘制调用
	// 交换链呈现目标。对于到其他目标的绘制调用，
	// 不应应用此转换。

	// 此示例使用行主序矩阵利用右手坐标系。
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// 眼睛位于(0,0.7,1.5)，并沿着 Y 轴使用向上矢量查找点(0,-0.1,0)。
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// 每个帧调用一次，旋转立方体，并计算模型和视图矩阵。
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (m_loadingComplete)
	{
		if (!m_tracking)
		{
			// 少量旋转立方体。
			m_angle += static_cast<float>(timer.GetElapsedSeconds()) * m_radiansPerSecond;

			Rotate(m_angle);
		}

		// 更新常量缓冲区资源。
		UINT8* destination = m_mappedConstantBuffer + (m_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

		// Update the ambient color
		m_constantPSBufferData.ambientColor = XMFLOAT4(0.31f, 0.31f, 0.31f,  1.0f); // light black
		m_constantPSBufferData.specularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // red directional red color
		m_constantPSBufferData.lightDirection = XMFLOAT3(-1.0f, -1.0f, -1.0f); // light in the z direction	
		m_constantPSBufferData.power = 15.0f;
		memcpy(m_mappedConstantPSBuffer, &m_constantPSBufferData, sizeof(m_constantPSBufferData));
	}
}

// 保存呈现器的当前状态。
void Sample3DSceneRenderer::SaveState()
{
	auto state = ApplicationData::Current->LocalSettings->Values;

	if (state->HasKey(AngleKey))
	{
		state->Remove(AngleKey);
	}
	if (state->HasKey(TrackingKey))
	{
		state->Remove(TrackingKey);
	}

	state->Insert(AngleKey, PropertyValue::CreateSingle(m_angle));
	state->Insert(TrackingKey, PropertyValue::CreateBoolean(m_tracking));
}

// 旋转呈现器的以前状态。
void Sample3DSceneRenderer::LoadState()
{
	auto state = ApplicationData::Current->LocalSettings->Values;
	if (state->HasKey(AngleKey))
	{
		m_angle = safe_cast<IPropertyValue^>(state->Lookup(AngleKey))->GetSingle();
		state->Remove(AngleKey);
	}
	if (state->HasKey(TrackingKey))
	{
		m_tracking = safe_cast<IPropertyValue^>(state->Lookup(TrackingKey))->GetBoolean();
		state->Remove(TrackingKey);
	}
}

// 将 3D 立方体模型旋转一定数量的弧度。
void Sample3DSceneRenderer::Rotate(float radians)
{
	// 准备将更新的模型矩阵传递到着色器。
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	m_constantBufferData.lightDirction = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// 进行跟踪时，可跟踪指针相对于输出屏幕宽度的位置，从而让 3D 立方体围绕其 Y 轴旋转。
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// 使用顶点和像素着色器呈现一个帧。
bool Sample3DSceneRenderer::Render()
{
	// 加载是异步的。仅在加载几何图形后才会绘制它。
	if (!m_loadingComplete)
	{
		return false;
	}

	DX::ThrowIfFailed(m_deviceResources->GetCommandAllocator()->Reset());

	// 调用 ExecuteCommandList() 后可随时重置命令列表。
	DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineState.Get()));

	PIXBeginEvent(m_commandList.Get(), 0, L"Draw the cube");
	{
		// 设置要由此帧使用的图形根签名和描述符堆。
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// 将当前帧的常量缓冲区绑定到管道。
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), m_deviceResources->GetCurrentFrameIndex(), m_cbvDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

		// Bind the current frame's texture to the pipeline.
		gpuHandle.InitOffsetted(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), DX::c_frameCount, m_cbvDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(1, gpuHandle);

		// Bind the current frame's texture buffer to the pipeline.
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle3(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), 4, m_cbvDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(2, gpuHandle3);

		// 设置视区和剪刀矩形。
		D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);

		// 指示此资源会用作呈现目标。
		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// 记录绘制命令。
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
		m_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		m_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->DrawIndexedInstanced(nIndices, 1, 0, 0, 0);

		// 指示呈现目标现在会用于展示命令列表完成执行的时间。
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(m_commandList.Get());

	DX::ThrowIfFailed(m_commandList->Close());

	// 执行命令列表。
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}


//
void CreateSphereVector(float radius, int slice, int stack, std::vector<VertexPositionColor> &VertexArray, std::vector<unsigned short> &indexArray)
{


	int vertsPerRow = slice + 1;
	int nRows = stack - 1;

	int nVerts = vertsPerRow * nRows + 2;
	int nIndices = (nRows - 1)*slice * 6 + slice * 6;

	VertexArray.clear();
	indexArray.clear();

	VertexArray.resize(nVerts);
	indexArray.resize(nIndices);

	for (int i = 1; i <= nRows; ++i)
	{
		float phy = XM_PI * i / stack;
		float tmpRadius = radius * sin(phy);
		for (int j = 0; j < vertsPerRow; ++j)
		{
			float theta = XM_2PI * j / slice;
			UINT index = (i - 1)*vertsPerRow + j;

			float x = tmpRadius*cos(theta);
			float y = radius*cos(phy);
			float z = tmpRadius*sin(theta);

			//位置坐标
			VertexArray[index].pos = XMFLOAT3(x, y, z);
			VertexArray[index].color = XMFLOAT3(0.3f, 0.3f, 0.3f);// XMFLOAT3(1.0f, 0.0f, 0.0f);
																  //法线
			XMVECTOR N = XMVectorSet(x, y, z, 0.f);
			XMStoreFloat3(&VertexArray[index].normal, XMVector3Normalize(N));
			//XMStoreFloat3(&mesh.vertices[index].normal, XMVector3Normalize(N));
			////切线
			//XMVECTOR T = XMVectorSet(-sin(theta), 0.f, cos(theta), 0.f);
			//XMStoreFloat3(&mesh.vertices[index].tangent, XMVector3Normalize(T));
			//纹理坐标
			VertexArray[index].uv = XMFLOAT2(j*1.f / slice, i*1.f / stack);
		}
	}

	int size = vertsPerRow * nRows;
	//添加顶部和底部两个顶点信息
	VertexArray[size].pos = XMFLOAT3(0.f, radius, 0.f);
	VertexArray[size].color = XMFLOAT3(0.3f, 0.3f, 0.3f); //XMFLOAT3(0.0f, 0.0f, 1.0f);
	VertexArray[size].normal = XMFLOAT3(0.f, 1.f, 0.f);
	//VertexArray[size].tangent = XMFLOAT3(1.f, 0.f, 0.f);
	VertexArray[size].uv = XMFLOAT2(0.f, 0.f);

	VertexArray[size + 1].pos = XMFLOAT3(0.f, -radius, 0.f);
	VertexArray[size + 1].color = XMFLOAT3(0.3f, 0.3f, 0.3f);// XMFLOAT3(0.0f, 0.0f, 1.0f);
	VertexArray[size + 1].normal = XMFLOAT3(0.f, -1.f, 0.f);
	//VertexArray[size + 1].tangent = XMFLOAT3(1.f, 0.f, 0.f);
	VertexArray[size + 1].uv = XMFLOAT2(0.f, 1.f);
	//sizeof(VertexArray) / sizeof(VertexArray[0])	
	UINT tmp(0);
	int start1 = 0;
	int start2 = nVerts - vertsPerRow - 2;
	int top = size;
	int bottom = size + 1;
	for (int i = 0; i < slice; ++i)
	{
		indexArray[tmp] = top;
		indexArray[tmp + 1] = start1 + i + 1;
		indexArray[tmp + 2] = start1 + i;

		tmp += 3;
	}

	for (int i = 0; i < slice; ++i)
	{
		indexArray[tmp] = bottom;
		indexArray[tmp + 1] = start2 + i;
		indexArray[tmp + 2] = start2 + i + 1;

		tmp += 3;
	}

	for (int i = 0; i < nRows - 1; ++i)
	{
		for (int j = 0; j < slice; ++j)
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
