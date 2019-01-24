#include "pch.h"
#include "PipelineStateManager.h"
#include "..\Common\DirectXHelper.h"


namespace DirectX12ColladaDaeMultipleMesh
{
	PipelineStateManager::PipelineStateManager()
	{
		m_pipelineStateMap.clear();
	}
	


	void PipelineStateManager::CreateGraphicsPipelineState(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		std::shared_ptr<DX::DeviceResources> deviceResources,
		std::function<void(HRESULT)> callBackFun)
	{		
			shared_ptr<PipelineStateInformationStruct> tempNormalMapStruct(new PipelineStateInformationStruct());
			shared_ptr<PipelineStateInformationStruct> tempCubeMapStruct(new PipelineStateInformationStruct());
			shared_ptr<PipelineStateInformationStruct> tempLightStruct(new PipelineStateInformationStruct());
#pragma region 创建管线状态部分
			// 通过异步方式加载着色器。
			auto createVSTask01 = DX::ReadDataAsync(L"NormalMapVertexShader.cso").then([this, tempNormalMapStruct](std::vector<byte>& fileData) {
				tempNormalMapStruct->m_vertexShader = fileData;
			});
			//CubeMapRefractPixelShader CubeMapRefractPixelShader
			auto createPSTask01 = DX::ReadDataAsync(L"NormalMapPixelShader.cso").then([this, tempNormalMapStruct](std::vector<byte>& fileData) {
				tempNormalMapStruct->m_pixelShader = fileData;
			});

			auto createVSTask02 = DX::ReadDataAsync(L"CubeMapVertexShader.cso").then([this, tempCubeMapStruct](std::vector<byte>& fileData) {
				tempCubeMapStruct->m_vertexShader = fileData;
			});

			auto createPSTask02 = DX::ReadDataAsync(L"CubeMapPixelShader.cso").then([this, tempCubeMapStruct](std::vector<byte>& fileData) {
				tempCubeMapStruct->m_pixelShader = fileData;
			});

			auto createVSTask03 = DX::ReadDataAsync(L"PlaneVertexShader.cso").then([this, tempLightStruct](std::vector<byte>& fileData) {
				tempLightStruct->m_vertexShader = fileData;
			});

			auto createPSTask03 = DX::ReadDataAsync(L"PlanePixelShader.cso").then([this, tempLightStruct](std::vector<byte>& fileData) {
				tempLightStruct->m_pixelShader = fileData;
			});


			// 加载着色器之后创建管道状态。
			auto createPipelineStateTask = 
				(createVSTask01 && createPSTask01 && createVSTask02 && createPSTask02 && createVSTask03 && createPSTask03).then(
					[this, rootSignature, deviceResources, tempNormalMapStruct, tempCubeMapStruct, tempLightStruct]()
			{


				static const D3D12_INPUT_ELEMENT_DESC inputLayoutForNormalMap[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
				state.InputLayout = { inputLayoutForNormalMap, _countof(inputLayoutForNormalMap) };
				state.pRootSignature = rootSignature.Get();
				state.VS = CD3DX12_SHADER_BYTECODE(&tempNormalMapStruct->m_vertexShader[0], tempNormalMapStruct->m_vertexShader.size());
				state.PS = CD3DX12_SHADER_BYTECODE(&tempNormalMapStruct->m_pixelShader[0], tempNormalMapStruct->m_pixelShader.size());
				state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				state.SampleMask = UINT_MAX;
				state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state.NumRenderTargets = 1;
				state.RTVFormats[0] = deviceResources->GetBackBufferFormat();
				state.DSVFormat = deviceResources->GetDepthBufferFormat();
				state.SampleDesc.Count = 1;

				// The camera is inside the sky sphere, so just turn off culling.
				state.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

				// Make sure the depth function is LESS_EQUAL and not just LESS.  
				// Otherwise, the normalized depth values at z = 1 (NDC) will 
				// fail the depth test if the depth buffer was cleared to 1.
				state.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

				//D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
				//transparencyBlendDesc.BlendEnable = true;
				//transparencyBlendDesc.LogicOpEnable = false;
				//transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				//transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				//transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				//transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				//transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
				//transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				//transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
				//transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				//state.BlendState.RenderTarget[0] = transparencyBlendDesc;

				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&tempNormalMapStruct->m_pipelineState)));

				// 创建管道状态之后可以删除着色器数据。
				tempNormalMapStruct->m_vertexShader.clear();
				tempNormalMapStruct->m_pixelShader.clear();

			

				static const D3D12_INPUT_ELEMENT_DESC inputLayoutForCubeMap[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state02 = {};
				state02.InputLayout = { inputLayoutForCubeMap, _countof(inputLayoutForCubeMap) };
				state02.pRootSignature = rootSignature.Get();
				state02.VS = CD3DX12_SHADER_BYTECODE(&tempCubeMapStruct->m_vertexShader[0], tempCubeMapStruct->m_vertexShader.size());
				state02.PS = CD3DX12_SHADER_BYTECODE(&tempCubeMapStruct->m_pixelShader[0], tempCubeMapStruct->m_pixelShader.size());
				state02.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state02.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state02.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				state02.SampleMask = UINT_MAX;
				state02.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state02.NumRenderTargets = 1;
				state02.RTVFormats[0] = deviceResources->GetBackBufferFormat();
				state02.DSVFormat = deviceResources->GetDepthBufferFormat();
				state02.SampleDesc.Count = 1;

				// The camera is inside the sky sphere, so just turn off culling.
				state02.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

				// Make sure the depth function is LESS_EQUAL and not just LESS.  
				// Otherwise, the normalized depth values at z = 1 (NDC) will 
				// fail the depth test if the depth buffer was cleared to 1.
				state02.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state02, IID_PPV_ARGS(&tempCubeMapStruct->m_pipelineState)));

				// 创建管道状态之后可以删除着色器数据。
				tempCubeMapStruct->m_vertexShader.clear();
				tempCubeMapStruct->m_pixelShader.clear();

				static const D3D12_INPUT_ELEMENT_DESC inputLayoutLight[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state03 = {};
				state03.InputLayout = { inputLayoutLight, _countof(inputLayoutLight) };
				state03.pRootSignature = rootSignature.Get();
				state03.VS = CD3DX12_SHADER_BYTECODE(&tempLightStruct->m_vertexShader[0], tempLightStruct->m_vertexShader.size());
				state03.PS = CD3DX12_SHADER_BYTECODE(&tempLightStruct->m_pixelShader[0], tempLightStruct->m_pixelShader.size());
				state03.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state03.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state03.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				state03.SampleMask = UINT_MAX;
				state03.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state03.NumRenderTargets = 1;
				state03.RTVFormats[0] = deviceResources->GetBackBufferFormat();
				state03.DSVFormat = deviceResources->GetDepthBufferFormat();
				state03.SampleDesc.Count = 1;



				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state03, IID_PPV_ARGS(&tempLightStruct->m_pipelineState)));

				// 创建管道状态之后可以删除着色器数据。
				tempLightStruct->m_vertexShader.clear();
				tempLightStruct->m_pixelShader.clear();

			});
#pragma endregion

			createPipelineStateTask.then([this, deviceResources, callBackFun, tempNormalMapStruct, tempCubeMapStruct, tempLightStruct]()
			{
				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateCommandList(
					0, D3D12_COMMAND_LIST_TYPE_DIRECT, deviceResources->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_commandList)));
				NAME_D3D12_OBJECT(m_commandList);
			
				m_pipelineStateMap.insert(pair<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>(L"NormalMapHLSLShader", tempNormalMapStruct));

				m_pipelineStateMap.insert(pair<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>(L"CubeMapHLSLShader", tempCubeMapStruct));

				m_pipelineStateMap.insert(pair<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>(L"LightHLSLShader", tempLightStruct));

				if (callBackFun != nullptr)
				{
					callBackFun(S_OK);
				}
			});

	}

}
