#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include <ppltasks.h>
#include <synchapi.h>

#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"

using namespace DiretctX12IndirectComputeShader;

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Storage;

// 应用程序状态映射中的索引。
Platform::String^ AngleKey = "Angle";
Platform::String^ TrackingKey = "Tracking";

const UINT Sample3DSceneRenderer::CommandSizePerFrame = SquareCount * sizeof(IndirectCommand);
const UINT Sample3DSceneRenderer::CommandBufferCounterOffset = AlignForUavCounter(Sample3DSceneRenderer::CommandSizePerFrame);
const float Sample3DSceneRenderer::SquareHalfWidth = 0.05f;
const float Sample3DSceneRenderer::SquareDepth = 1.0f;
const float Sample3DSceneRenderer::CullingCutoff = 0.5f;

// 从文件中加载顶点和像素着色器，然后实例化立方体几何图形。
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_radiansPerSecond(XM_PIDIV4),	// 每秒旋转 45 度
	m_angle(0),
	m_tracking(false),
	//m_mappedConstantBuffer(nullptr),
	m_deviceResources(deviceResources),
	m_cullingScissorRect(),
	m_enableCulling(true)
{
	LoadState();
	//ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));
	m_constantBufferData.resize(SquareCount);

	m_csRootConstants.xOffset = SquareHalfWidth;//.....computerShader
	m_csRootConstants.zOffset = SquareDepth;//.....computerShader
	m_csRootConstants.cullOffset = CullingCutoff;//.....computerShader
	m_csRootConstants.commandCount = SquareCount;//.....computerShader

	Size outputSize = m_deviceResources->GetOutputSize();
	//float aspectRatio = outputSize.Width / outputSize.Height;
	float center = outputSize.Width / 2.0f;
	m_cullingScissorRect.left = static_cast<LONG>(center - (center * CullingCutoff));
	m_cullingScissorRect.right = static_cast<LONG>(center + (center * CullingCutoff));
	m_cullingScissorRect.bottom = static_cast<LONG>(outputSize.Height);

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();


}

Sample3DSceneRenderer::~Sample3DSceneRenderer()
{
	//m_constantBuffer->Unmap(0, nullptr);
	//m_mappedConstantBuffer = nullptr;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// 创建立方体几何图形资源并上载到 GPU。
	auto createAssetslambda = [this]() {

	auto d3dDevice = m_deviceResources->GetD3DDevice();

	//-------------------------compute---------------start
	// Describe and create the command queues.
	D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
	computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

	// Create frame resources.
	DX::ThrowIfFailed(d3dDevice->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&m_computeCommandQueue)));
	NAME_D3D12_OBJECT(m_computeCommandQueue);

	for (int n = 0; n < DX::c_frameCount; n++)
	{
		DX::ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_computeCommandAllocators[n])));			
	}

	
		//-------------------------compute---------------end
	//---------------------------666------------------------------------------------

	// Describe and create a constant buffer view (CBV), Shader resource
	// view (SRV), and unordered access view (UAV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = CbvSrvUavDescriptorCountPerFrame * DX::c_frameCount + 1; // for texture;;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));
	NAME_D3D12_OBJECT(m_cbvSrvUavHeap);

	m_cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 创建具有单个常量缓冲区槽的根签名。
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		//--------------
		CD3DX12_DESCRIPTOR_RANGE1 texRanges[1];
		texRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//-----------------------------------

		CD3DX12_ROOT_PARAMETER1 rootParameters[GraphicsRootParametersCount];
		rootParameters[Cbv].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[Tex].InitAsDescriptorTable(1, &texRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		//----------------


		//CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//------------

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		DX::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		NAME_D3D12_OBJECT(m_rootSignature);

		//-------------------------compute---------------start
		// Create compute signature.
		D3D12_DESCRIPTOR_RANGE1 ranges[2];
		ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		ranges[0].NumDescriptors = 2;
		ranges[0].BaseShaderRegister = 0;
		ranges[0].RegisterSpace = 0;
		ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
		ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		ranges[1].NumDescriptors = 1;
		ranges[1].BaseShaderRegister = 0;
		ranges[1].RegisterSpace = 0;
		ranges[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
		ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		D3D12_ROOT_PARAMETER1 computeRootParameters[ComputeRootParametersCount];
		computeRootParameters[SrvUavTable].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		computeRootParameters[SrvUavTable].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		computeRootParameters[SrvUavTable].DescriptorTable.NumDescriptorRanges =2;
		computeRootParameters[SrvUavTable].DescriptorTable.pDescriptorRanges = ranges;
		
		computeRootParameters[RootConstants].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		computeRootParameters[RootConstants].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		computeRootParameters[RootConstants].Constants.Num32BitValues = 4;
		computeRootParameters[RootConstants].Constants.ShaderRegister = 0;
		computeRootParameters[RootConstants].Constants.RegisterSpace = 0;
		//CD3DX12_ROOT_PARAMETER1 computeRootParameters[ComputeRootParametersCount];
		//computeRootParameters[SrvUavTable].InitAsDescriptorTable(2, ranges);
		//computeRootParameters[RootConstants].InitAsConstants(4, 0);

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
		computeRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		computeRootSignatureDesc.Desc_1_1.NumParameters = _countof(computeRootParameters);
		computeRootSignatureDesc.Desc_1_1.pParameters = computeRootParameters;
		computeRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
		computeRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
		computeRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
		//computeRootSignatureDesc.Init_1_1(_countof(computeRootParameters), computeRootParameters);
		//D3D_ROOT_SIGNATURE_VERSION_1
		               
		//DX::ThrowIfFailed(D3D12SerializeRootSignature(&computeRootSignatureDesc.Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));
		DX::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&computeRootSignatureDesc, featureData.HighestVersion, &signature, &error));
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature)));
		NAME_D3D12_OBJECT(m_computeRootSignature);

		//-------------------------compute---------------end

		//CD3DX12_DESCRIPTOR_RANGE range;
		//CD3DX12_ROOT_PARAMETER parameter;

		//range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		//parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		//D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // 只有输入汇编程序阶段才需要访问常量缓冲区。
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		//CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		//descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

		//ComPtr<ID3DBlob> pSignature;
		//ComPtr<ID3DBlob> pError;
		//DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
		//DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
  //      NAME_D3D12_OBJECT(m_rootSignature);
	}

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	ComPtr<ID3DBlob> computeShader;
	ComPtr<ID3DBlob> error;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	std::wstring m_assetsPath = assetsPath;
	std::wstring filePathVS = m_assetsPath + L"Assets\\shaders.hlsl";
	std::wstring filePathPS = m_assetsPath + L"Assets\\shaders.hlsl";
	std::wstring filePathCS = m_assetsPath + L"Assets\\compute.hlsl";
	DX::ThrowIfFailed(D3DCompileFromFile(filePathVS.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &error));
	DX::ThrowIfFailed(D3DCompileFromFile(filePathPS.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &error));
	DX::ThrowIfFailed(D3DCompileFromFile(filePathCS.c_str(), nullptr, nullptr, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &error));

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state objects (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;

	DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	NAME_D3D12_OBJECT(m_pipelineState);

	//-------------------------compute---------------start
	// Describe and create the compute pipeline state object (PSO).
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = m_computeRootSignature.Get();
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
	DX::ThrowIfFailed(d3dDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_computeState)));
	NAME_D3D12_OBJECT(m_computeState);
	//-------------------------compute---------------end

	DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	NAME_D3D12_OBJECT(m_commandList);

	//-------------------------compute---------------start
	DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeCommandAllocators[m_deviceResources->GetCurrentFrameIndex()].Get(), m_computeState.Get(), IID_PPV_ARGS(&m_computeCommandList)));

	DX::ThrowIfFailed(m_computeCommandList->Close());
	NAME_D3D12_OBJECT(m_computeCommandList);
	//-------------------------compute---------------end
	//---------------------------------------------------------------------fff----------------
	// Note: ComPtr's are CPU objects but these resources need to stay in scope until
	// the command list that references them has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resources are not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> vertexBufferUpload;
	ComPtr<ID3D12Resource> commandBufferUpload;

	// Create the vertex buffer.
	{
		// Define the geometry for a square.
		Vertex triangleVertices[] =
		{
			//{ { 0.0f, SquareHalfWidth, SquareDepth },{ 0.2f, 0.2f } },
			//{ { SquareHalfWidth, -SquareHalfWidth, SquareDepth },{ 0.2f, 0.8f } },
			//{ { -SquareHalfWidth, -SquareHalfWidth, SquareDepth },{ 0.8f, 0.2f } },

			//{ { SquareHalfWidth, -SquareHalfWidth, SquareDepth } },
			//{ { 0.0f, SquareHalfWidth, SquareDepth } },
			//{ { SquareHalfWidth*2.0f, SquareHalfWidth, SquareDepth } },
			
			{ { SquareHalfWidth,SquareHalfWidth, SquareDepth },{ 1.0f, 0.0f } },
			{ { -SquareHalfWidth, SquareHalfWidth, SquareDepth },{ 0.0f, 0.0f } },
			{ { -SquareHalfWidth,-SquareHalfWidth, SquareDepth } ,{ 0.0f, 1.0f } },


			{ { SquareHalfWidth,SquareHalfWidth, SquareDepth },{ 1.0f, 0.0f } },
			{ { -SquareHalfWidth,-SquareHalfWidth, SquareDepth },{ 0.0f, 1.0f } },
			{ { SquareHalfWidth,-SquareHalfWidth, SquareDepth },{ 1.0f, 1.0f } },
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));

		NAME_D3D12_OBJECT(m_vertexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<UINT8*>(triangleVertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	}


	//---------------------------------------------------------------------fff----------------
	// Create the constant buffers.
	{
		const UINT constantBufferDataSize = SquareResourceCount * sizeof(SceneConstantBuffer);// ;

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		NAME_D3D12_OBJECT(m_constantBuffer);

		Size outputSize = m_deviceResources->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		// Initialize the constant buffers for each of the color squares.
		for (UINT n = 0; n < SquareCount; n++)
		{
			m_constantBufferData[n].velocity = XMFLOAT4(GetRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
			m_constantBufferData[n].offset = XMFLOAT4(GetRandomFloat(-5.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
			m_constantBufferData[n].color = XMFLOAT4(GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), 1.0f);
			XMStoreFloat4x4(&m_constantBufferData[n].projection, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.01f, 20.0f)));
		}

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, &m_constantBufferData[0], SquareCount * sizeof(SceneConstantBuffer));//

		// Create shader resource views (SRV) of the constant buffers for the
		// compute shader to read from.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = SquareCount;
		srvDesc.Buffer.StructureByteStride =  sizeof(SceneConstantBuffer);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), CbvSrvOffset, m_cbvSrvUavDescriptorSize);
		for (UINT frame = 0; frame < DX::c_frameCount; frame++)
		{
			srvDesc.Buffer.FirstElement = frame * SquareCount;
			d3dDevice->CreateShaderResourceView(m_constantBuffer.Get(), &srvDesc, cbvSrvHandle);
			cbvSrvHandle.Offset(CbvSrvUavDescriptorCountPerFrame, m_cbvSrvUavDescriptorSize);
		}
	}


	// Create the command signature used for indirect drawing.
	{
		// Each command consists of a CBV update and a DrawInstanced call.
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		argumentDescs[0].ConstantBufferView.RootParameterIndex = Cbv;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

		DX::ThrowIfFailed(d3dDevice->CreateCommandSignature(&commandSignatureDesc, m_rootSignature.Get(), IID_PPV_ARGS(&m_commandSignature)));
		NAME_D3D12_OBJECT(m_commandSignature);
	}





	//---------------------------------------------------------------------fff----------------

	// Create the command buffers and UAVs to store the results of the compute work.
	{
		std::vector<IndirectCommand> commands;
		commands.resize(SquareResourceCount);
		const UINT commandBufferSize = CommandSizePerFrame * DX::c_frameCount;

		D3D12_RESOURCE_DESC commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(commandBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&commandBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_commandBuffer)));

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(commandBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&commandBufferUpload)));

		NAME_D3D12_OBJECT(m_commandBuffer);

		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_constantBuffer->GetGPUVirtualAddress();
		UINT commandIndex = 0;

		for (UINT frame = 0; frame < DX::c_frameCount; frame++)
		{
			for (UINT n = 0; n < SquareCount; n++)
			{
				commands[commandIndex].cbv = gpuAddress;
				commands[commandIndex].drawArguments.VertexCountPerInstance = 6;// 3;
				commands[commandIndex].drawArguments.InstanceCount = 1;//1
				commands[commandIndex].drawArguments.StartVertexLocation = 0;
				commands[commandIndex].drawArguments.StartInstanceLocation = 0;

				commandIndex++;
				gpuAddress +=  sizeof(SceneConstantBuffer);
			}
		}

		// Copy data to the intermediate upload heap and then schedule a copy
		// from the upload heap to the command buffer.
		D3D12_SUBRESOURCE_DATA commandData = {};
		commandData.pData = reinterpret_cast<UINT8*>(&commands[0]);
		commandData.RowPitch = commandBufferSize;
		commandData.SlicePitch = commandData.RowPitch;

		UpdateSubresources<1>(m_commandList.Get(), m_commandBuffer.Get(), commandBufferUpload.Get(), 0, 0, 1, &commandData);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_commandBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		// Create SRVs for the command buffers.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = SquareCount;
		srvDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE commandsHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), CommandsOffset, m_cbvSrvUavDescriptorSize);
		for (UINT frame = 0; frame < DX::c_frameCount; frame++)
		{
			srvDesc.Buffer.FirstElement = frame * SquareCount;
			d3dDevice->CreateShaderResourceView(m_commandBuffer.Get(), &srvDesc, commandsHandle);
			commandsHandle.Offset(CbvSrvUavDescriptorCountPerFrame, m_cbvSrvUavDescriptorSize);
		}






		//-------------------------compute---------------start
		// Create the unordered access views (UAVs) that store the results of the compute work.
		CD3DX12_CPU_DESCRIPTOR_HANDLE processedCommandsHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), ProcessedCommandsOffset, m_cbvSrvUavDescriptorSize);
		for (UINT frame = 0; frame < DX::c_frameCount; frame++)
		{
			// Allocate a buffer large enough to hold all of the indirect commands
			// for a single frame as well as a UAV counter.
			commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CommandBufferCounterOffset + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&commandBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_processedCommandBuffers[frame])));

			//NAME_D3D12_OBJECT_INDEXED(m_processedCommandBuffers, frame);

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = SquareCount;
			uavDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
			uavDesc.Buffer.CounterOffsetInBytes = CommandBufferCounterOffset;
			
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			d3dDevice->CreateUnorderedAccessView(
				m_processedCommandBuffers[frame].Get(),
				m_processedCommandBuffers[frame].Get(),
				&uavDesc,
				processedCommandsHandle);

			processedCommandsHandle.Offset(CbvSrvUavDescriptorCountPerFrame, m_cbvSrvUavDescriptorSize);
		}

		// Allocate a buffer that can be used to reset the UAV counters and initialize
		// it to 0.
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_processedCommandBufferCounterReset)));

		UINT8* pMappedCounterReset = nullptr;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		DX::ThrowIfFailed(m_processedCommandBufferCounterReset->Map(0, &readRange, reinterpret_cast<void**>(&pMappedCounterReset)));
		ZeroMemory(pMappedCounterReset, sizeof(UINT));
		m_processedCommandBufferCounterReset->Unmap(0, nullptr);
		//-------------------------compute---------------end

}


#pragma region 加载纹理部分
LPCWSTR szFile = L"Assets\\new_icon.png";//

DWORD flags = CP_FLAGS::CP_FLAGS_NONE;
ScratchImage image;
DirectX::LoadFromWICFile(szFile, flags, nullptr, image);

auto metadata1 = image.GetMetadata();
auto texture = image.GetPixels();

//CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::
//Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, metadata1.width, metadata1.height, 1, 1);

// Describe and create a Texture2D.
D3D12_RESOURCE_DESC textureDesc = {};
textureDesc.MipLevels = 1;
textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
textureDesc.Width = metadata1.width;
textureDesc.Height = metadata1.height;
textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
textureDesc.DepthOrArraySize = 1;
textureDesc.SampleDesc.Count = 1;
textureDesc.SampleDesc.Quality = 0;
textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	D3D12_HEAP_FLAG_NONE,
	&textureDesc,
	D3D12_RESOURCE_STATE_COPY_DEST, //D3D12_RESOURCE_STATE_COMMON,
	nullptr,
	IID_PPV_ARGS(&m_texture)));

ComPtr<ID3D12Resource> uploadResources;
//const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1) + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);
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
	textureData.RowPitch =  image.GetImages()->rowPitch;
	textureData.SlicePitch = textureData.RowPitch * metadata1.height;

	UpdateSubresources(m_commandList.Get(), m_texture.Get(), uploadResources.Get(), 0, 0, 1, &textureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	//UpdateSubresources(m_commandList.Get(), m_texture.Get(), uploadResources.Get(), 0, 0, 1, &textureData);

	//UpdateSubresources<1>(m_commandList.Get(), m_texture.Get(), uploadResources.Get(), 0, 0, 1, &textureData);
	//m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));


	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;// textureDesc.MipLevels;

	CD3DX12_CPU_DESCRIPTOR_HANDLE texHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), CbvSrvUavDescriptorCountPerFrame * DX::c_frameCount, m_cbvSrvUavDescriptorSize);
	d3dDevice->CreateShaderResourceView(m_texture.Get(), &srvDesc, texHandle);
	texHandle.Offset(m_cbvSrvUavDescriptorSize);
}
#pragma endregion


	// Close the command list and execute it to begin the vertex buffer copy into
	// the default heap.
	DX::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//-------------------------compute---------------start
	DX::ThrowIfFailed(d3dDevice->CreateFence(m_deviceResources->GetCurFanceValue(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_computeFence)));
	//-------------------------compute---------------end
		// 等待命令列表完成执行；顶点/索引缓冲区需要在上载资源超出范围之前上载到 GPU。
		m_deviceResources->WaitForGpu();
	};

	auto createAssetsTask = create_task(createAssetslambda);
	createAssetsTask.then([this]() {
	
		m_loadingComplete = true;
	});

		
	//// 通过异步方式加载着色器。
	//auto createVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso").then([this](std::vector<byte>& fileData) {
	//	m_vertexShader = fileData;
	//});

	//auto createPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso").then([this](std::vector<byte>& fileData) {
	//	m_pixelShader = fileData;
	//});

	//// 加载着色器之后创建管道状态。
	//auto createPipelineStateTask = (createPSTask && createVSTask).then([this]() {

	//	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	};

	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	//	state.InputLayout = { inputLayout, _countof(inputLayout) };
	//	state.pRootSignature = m_rootSignature.Get();
 //       state.VS = CD3DX12_SHADER_BYTECODE(&m_vertexShader[0], m_vertexShader.size());
 //       state.PS = CD3DX12_SHADER_BYTECODE(&m_pixelShader[0], m_pixelShader.size());
	//	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//	state.SampleMask = UINT_MAX;
	//	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	state.NumRenderTargets = 1;
	//	state.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
	//	state.DSVFormat = m_deviceResources->GetDepthBufferFormat();
	//	state.SampleDesc.Count = 1;

	//	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&m_pipelineState)));

	//	// 创建管道状态之后可以删除着色器数据。
	//	m_vertexShader.clear();
	//	m_pixelShader.clear();
	//});

	//// 创建立方体几何图形资源并上载到 GPU。
	//auto createAssetsTask = createPipelineStateTask.then([this]() {
	//	auto d3dDevice = m_deviceResources->GetD3DDevice();

	//	// 创建命令列表。
	//	DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
 //       NAME_D3D12_OBJECT(m_commandList);

	//	// 立方体顶点。每个顶点都有一个位置和一个颜色。
	//	VertexPositionColor cubeVertices[] =
	//	{
	//		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
	//		{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	//		{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	//		{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
	//		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
	//		{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
	//		{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
	//		{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
	//	};

	//	const UINT vertexBufferSize = sizeof(cubeVertices);

	//	// 在 GPU 的默认堆中创建顶点缓冲区资源并使用上载堆将顶点数据复制到其中。
	//	// 在 GPU 使用完之前，不得释放上载资源。
	//	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

	//	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	//	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	//	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//		&defaultHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&vertexBufferDesc,
	//		D3D12_RESOURCE_STATE_COPY_DEST,
	//		nullptr,
	//		IID_PPV_ARGS(&m_vertexBuffer)));

	//	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	//	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//		&uploadHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&vertexBufferDesc,
	//		D3D12_RESOURCE_STATE_GENERIC_READ,
	//		nullptr,
	//		IID_PPV_ARGS(&vertexBufferUpload)));

 //       NAME_D3D12_OBJECT(m_vertexBuffer);

	//	// 将顶点缓冲区上载到 GPU。
	//	{
	//		D3D12_SUBRESOURCE_DATA vertexData = {};
	//		vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
	//		vertexData.RowPitch = vertexBufferSize;
	//		vertexData.SlicePitch = vertexData.RowPitch;

	//		UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

	//		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
	//			CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	//		m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	//	}

	//	// 加载网格索引。每三个索引表示要在屏幕上呈现的三角形。
	//	// 例如: 0,2,1 表示顶点缓冲区中的索引为 0、2 和 1 的顶点构成
	//	// 此网格的第一个三角形。
	//	unsigned short cubeIndices[] =
	//	{
	//		0, 2, 1, // -x
	//		1, 2, 3,

	//		4, 5, 6, // +x
	//		5, 7, 6,

	//		0, 1, 5, // -y
	//		0, 5, 4,

	//		2, 6, 7, // +y
	//		2, 7, 3,

	//		0, 4, 6, // -z
	//		0, 6, 2,

	//		1, 3, 7, // +z
	//		1, 7, 5,
	//	};

	//	const UINT indexBufferSize = sizeof(cubeIndices);

	//	// 在 GPU 的默认堆中创建索引缓冲区资源并使用上载堆将索引数据复制到其中。
	//	// 在 GPU 使用完之前，不得释放上载资源。
	//	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

	//	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	//	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//		&defaultHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&indexBufferDesc,
	//		D3D12_RESOURCE_STATE_COPY_DEST,
	//		nullptr,
	//		IID_PPV_ARGS(&m_indexBuffer)));

	//	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//		&uploadHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&indexBufferDesc,
	//		D3D12_RESOURCE_STATE_GENERIC_READ,
	//		nullptr,
	//		IID_PPV_ARGS(&indexBufferUpload)));

	//	NAME_D3D12_OBJECT(m_indexBuffer);

	//	// 将索引缓冲区上载到 GPU。
	//	{
	//		D3D12_SUBRESOURCE_DATA indexData = {};
	//		indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
	//		indexData.RowPitch = indexBufferSize;
	//		indexData.SlicePitch = indexData.RowPitch;

	//		UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

	//		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
	//			CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	//		m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	//	}

	//	// 为常量缓冲区创建描述符堆。
	//	{
	//		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//		heapDesc.NumDescriptors = DX::c_frameCount;
	//		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//		// 此标志指示此描述符堆可以绑定到管道，并且其中包含的描述符可以由根表引用。
	//		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//		DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

 //           NAME_D3D12_OBJECT(m_cbvHeap);
	//	}

	//	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(DX::c_frameCount * c_alignedConstantBufferSize);
	//	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
	//		&uploadHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&constantBufferDesc,
	//		D3D12_RESOURCE_STATE_GENERIC_READ,
	//		nullptr,
	//		IID_PPV_ARGS(&m_constantBuffer)));

 //       NAME_D3D12_OBJECT(m_constantBuffer);

	//	// 创建常量缓冲区视图以访问上载缓冲区。
	//	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
	//	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
	//	m_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//	for (int n = 0; n < DX::c_frameCount; n++)
	//	{
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	//		desc.BufferLocation = cbvGpuAddress;
	//		desc.SizeInBytes = c_alignedConstantBufferSize;
	//		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

	//		cbvGpuAddress += desc.SizeInBytes;
	//		cbvCpuHandle.Offset(m_cbvDescriptorSize);
	//	}

	//	// 映射常量缓冲区。
	//	CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
	//	DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
	//	ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount * c_alignedConstantBufferSize);
	//	// 应用关闭之前，我们不会对此取消映射。在资源生命周期内使对象保持映射状态是可行的。

	//	// 关闭命令列表并执行它，以开始将顶点/索引缓冲区复制到 GPU 的默认堆中。
	//	DX::ThrowIfFailed(m_commandList->Close());
	//	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	//	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//	// 创建顶点/索引缓冲区视图。
	//	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	//	m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
	//	m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

	//	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	//	m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
	//	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	//	// 等待命令列表完成执行；顶点/索引缓冲区需要在上载资源超出范围之前上载到 GPU。
	//	m_deviceResources->WaitForGpu();
	//});

	//createAssetsTask.then([this]() {
	//	m_loadingComplete = true;
	//});
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

	//// 请注意，OrientationTransform3D 矩阵在此处是后乘的，
	//// 以正确确定场景的方向，使之与显示方向匹配。
	//// 对于交换链的目标位图进行的任何绘制调用
	//// 交换链呈现目标。对于到其他目标的绘制调用，
	//// 不应应用此转换。

	//// 此示例使用行主序矩阵利用右手坐标系。
	//XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
	//	fovAngleY,
	//	aspectRatio,
	//	0.01f,
	//	100.0f
	//	);

	//XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	//XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	//XMStoreFloat4x4(
	//	&m_constantBufferData.projection,
	//	XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	//	);

	//// 眼睛位于(0,0.7,1.5)，并沿着 Y 轴使用向上矢量查找点(0,-0.1,0)。
	//static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	//static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	//static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	//XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

//	Size outputSize = m_deviceResources->GetOutputSize();
	//float aspectRatio = outputSize.Width / outputSize.Height;
	float center = outputSize.Width / 2.0f;
	m_cullingScissorRect.left = static_cast<LONG>(center - (center * CullingCutoff));
	m_cullingScissorRect.right = static_cast<LONG>(center + (center * CullingCutoff));
	m_cullingScissorRect.bottom = static_cast<LONG>(outputSize.Height);

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

		//// 更新常量缓冲区资源。
		//UINT8* destination = m_mappedConstantBuffer + (m_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		//memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

		for (UINT n = 0; n < SquareCount; n++)
		{
			const float offsetBounds = 2.5f;

			// Animate the color squares.
			m_constantBufferData[n].offset.x += m_constantBufferData[n].velocity.x;
			if (m_constantBufferData[n].offset.x > offsetBounds)
			{
				m_constantBufferData[n].velocity.x = GetRandomFloat(0.01f, 0.02f);
				m_constantBufferData[n].offset.x = -offsetBounds;
			}
		}

		UINT8* destination = m_pCbvDataBegin + (SquareCount * m_deviceResources->GetCurrentFrameIndex() * sizeof(SceneConstantBuffer));//
		memcpy(destination, &m_constantBufferData[0], SquareCount * sizeof(SceneConstantBuffer));//
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
	//XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
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

	//// Command list allocators can only be reset when the associated 
	//// command lists have finished execution on the GPU; apps should use 
	//// fences to determine GPU execution progress.
	//-------------------------compute---------------start
	DX::ThrowIfFailed(m_computeCommandAllocators[m_deviceResources->GetCurrentFrameIndex()]->Reset());
	//-------------------------compute---------------end
	DX::ThrowIfFailed(m_deviceResources->GetCommandAllocator()->Reset());

	//// However, when ExecuteCommandList() is called on a particular command 
	//// list, that command list can then be reset at any time and must be before 
	//// re-recording.
	//-------------------------compute---------------start
	DX::ThrowIfFailed(m_computeCommandList->Reset(m_computeCommandAllocators[m_deviceResources->GetCurrentFrameIndex()].Get(), m_computeState.Get()));
	//-------------------------compute---------------end
	DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineState.Get()));

	//-------------------------compute---------------start
	// Record the compute commands that will cull color squares and prevent them from being processed by the vertex shader.
	if (m_enableCulling)
	{
		UINT frameDescriptorOffset = m_deviceResources->GetCurrentFrameIndex() * CbvSrvUavDescriptorCountPerFrame;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHandle = m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

		m_computeCommandList->SetComputeRootSignature(m_computeRootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
		m_computeCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE computeHandle(cbvSrvUavHandle, CbvSrvOffset + frameDescriptorOffset, m_cbvSrvUavDescriptorSize);
		m_computeCommandList->SetComputeRootDescriptorTable(
			SrvUavTable,
			//CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvSrvUavHandle, CbvSrvOffset + frameDescriptorOffset, m_cbvSrvUavDescriptorSize));
			computeHandle);

		m_computeCommandList->SetComputeRoot32BitConstants(RootConstants, 4, reinterpret_cast<void*>(&m_csRootConstants), 0);

		// Reset the UAV counter for this frame.
		m_computeCommandList->CopyBufferRegion(m_processedCommandBuffers[m_deviceResources->GetCurrentFrameIndex()].Get(), CommandBufferCounterOffset, m_processedCommandBufferCounterReset.Get(), 0, sizeof(UINT));

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_processedCommandBuffers[m_deviceResources->GetCurrentFrameIndex()].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_computeCommandList->ResourceBarrier(1, &barrier);

		m_computeCommandList->Dispatch(static_cast<UINT>(ceil(SquareCount / float(ComputeThreadBlockSize))), 1, 1);
	}

	DX::ThrowIfFailed(m_computeCommandList->Close());
	//-------------------------compute---------------end

	// Record the rendering commands.
	{
		// Set necessary state.
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), CbvSrvUavDescriptorCountPerFrame * DX::c_frameCount, m_cbvSrvUavDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(1, gpuHandle);


			// 设置视区和剪刀矩形。
			D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
			m_commandList->RSSetViewports(1, &viewport);
			//m_commandList->RSSetScissorRects(1, &m_scissorRect);
			m_commandList->RSSetScissorRects(1, m_enableCulling ? &m_cullingScissorRect : &m_scissorRect);
	
			
			

		// Indicate that the command buffer will be used for indirect drawing
		// and that the back buffer will be used as a render target.
		//D3D12_RESOURCE_BARRIER barriers[2] = {
		//	CD3DX12_RESOURCE_BARRIER::Transition(
		//		 m_commandBuffer.Get(),
		//		 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		//		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT),
		//	CD3DX12_RESOURCE_BARRIER::Transition(
		//		m_deviceResources->GetRenderTarget(),
		//		D3D12_RESOURCE_STATE_PRESENT,
		//		D3D12_RESOURCE_STATE_RENDER_TARGET)
		//};

			D3D12_RESOURCE_BARRIER barriers[2] = {
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_enableCulling ? m_processedCommandBuffers[m_deviceResources->GetCurrentFrameIndex()].Get() : m_commandBuffer.Get(),
					m_enableCulling ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT),
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_deviceResources->GetRenderTarget(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RENDER_TARGET)
			};

		m_commandList->ResourceBarrier(_countof(barriers), barriers);


		//auto d3dDevice = m_deviceResources->GetD3DDevice();


		//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		//	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		//	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		//m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		//m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//// 记录绘制命令。
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
		m_commandList->ClearRenderTargetView(renderTargetView, clearColor, 0, nullptr);//DirectX::Colors::CornflowerBlue
		m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

		if (m_enableCulling)
		{
			PIXBeginEvent(m_commandList.Get(), 0, L"Draw visible color squares");

			// Draw the color squares that have not been culled.
			m_commandList->ExecuteIndirect(
				m_commandSignature.Get(),
				SquareCount,
				m_processedCommandBuffers[m_deviceResources->GetCurrentFrameIndex()].Get(),
				0,
				m_processedCommandBuffers[m_deviceResources->GetCurrentFrameIndex()].Get(),
				CommandBufferCounterOffset);
		}
		else
		{
			PIXBeginEvent(m_commandList.Get(), 0, L"Draw all color squares");

			// Draw all of the color squares.
			m_commandList->ExecuteIndirect(
				m_commandSignature.Get(),
				SquareCount,
				m_commandBuffer.Get(),
				CommandSizePerFrame * m_deviceResources->GetCurrentFrameIndex(),
				nullptr,
				0);
		}
		PIXEndEvent(m_commandList.Get());

		// Indicate that the command buffer may be used by the compute shader
		// and that the back buffer will now be used to present.
		barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		//barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barriers[0].Transition.StateAfter = m_enableCulling ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		m_commandList->ResourceBarrier(_countof(barriers), barriers);

		DX::ThrowIfFailed(m_commandList->Close());
	}

	//-------------------------compute---------------start
	// Execute the compute work.
	if (m_enableCulling)
	{
		PIXBeginEvent(m_deviceResources->GetCommandQueue(), 0, L"Cull invisible color squares");

		ID3D12CommandList* ppComputeCommandLists[] = { m_computeCommandList.Get() };
		m_computeCommandQueue->ExecuteCommandLists(_countof(ppComputeCommandLists), ppComputeCommandLists);

		PIXEndEvent(m_deviceResources->GetCommandQueue());

		m_computeCommandQueue->Signal(m_computeFence.Get(), m_deviceResources->GetCurFanceValue());

		// Execute the rendering work only when the compute work is complete.
		m_deviceResources->GetCommandQueue()->Wait(m_computeFence.Get(), m_deviceResources->GetCurFanceValue());
	}
	//-------------------------compute---------------end

	// 执行命令列表。
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/*DX::ThrowIfFailed(m_deviceResources->GetCommandAllocator()->Reset());*/

	//// 调用 ExecuteCommandList() 后可随时重置命令列表。
	//DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineState.Get()));

	//PIXBeginEvent(m_commandList.Get(), 0, L"Draw the cube");
	//{
	//	// 设置要由此帧使用的图形根签名和描述符堆。
	//	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	//	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
	//	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//	// 将当前帧的常量缓冲区绑定到管道。
	//	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), m_deviceResources->GetCurrentFrameIndex(), m_cbvDescriptorSize);
	//	m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	//	// 设置视区和剪刀矩形。
	//	D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
	//	m_commandList->RSSetViewports(1, &viewport);
	//	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	//	// 指示此资源会用作呈现目标。
	//	CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
	//		CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//	m_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

	//	// 记录绘制命令。
	//	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
	//	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
	//	m_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
	//	m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//	m_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

	//	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	//	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	//	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	//	// 指示呈现目标现在会用于展示命令列表完成执行的时间。
	//	CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
	//		CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//	m_commandList->ResourceBarrier(1, &presentResourceBarrier);
	//}
	//PIXEndEvent(m_commandList.Get());

	//DX::ThrowIfFailed(m_commandList->Close());

	//// 执行命令列表。
	//ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	//m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}
