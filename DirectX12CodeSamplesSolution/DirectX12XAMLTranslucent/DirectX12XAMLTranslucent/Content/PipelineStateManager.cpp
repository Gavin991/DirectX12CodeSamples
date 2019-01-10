#include "pch.h"
#include "PipelineStateManager.h"
#include "..\Common\DirectXHelper.h"
//#include "G:\Mybooks\UWPSamplesDirectX12\DirectXTex-master\DirectXTex\DirectXTex.h"

namespace DirectX12XAMLTranslucent
{
	PipelineStateManager::PipelineStateManager()
	{
		m_pipelineStateMap.clear();
	}
	


	void PipelineStateManager::CreateGraphicsPipelineState(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		std::shared_ptr<DX::DeviceResources> deviceResources,
		std::function<void(bool)> callBackFun)
	{		
			shared_ptr<PipelineStateInformationStruct> tempStruct01(new PipelineStateInformationStruct());
			shared_ptr<PipelineStateInformationStruct> tempStruct02(new PipelineStateInformationStruct());
#pragma region 创建管线状态部分
			// 通过异步方式加载着色器。
			auto createVSTask01 = DX::ReadDataAsync(L"SampleVertexShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
				tempStruct01->m_vertexShader = fileData;
			});

			auto createPSTask01 = DX::ReadDataAsync(L"SamplePixelShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
				tempStruct01->m_pixelShader = fileData;
			});

			auto createVSTask02 = DX::ReadDataAsync(L"PlaneVertexShader.cso").then([this, tempStruct02](std::vector<byte>& fileData) {
				tempStruct02->m_vertexShader = fileData;
			});

			auto createPSTask02 = DX::ReadDataAsync(L"PlanePixelShader.cso").then([this, tempStruct02](std::vector<byte>& fileData) {
				tempStruct02->m_pixelShader = fileData;
			});

			// 加载着色器之后创建管道状态。
			auto createPipelineStateTask = (createVSTask01 && createPSTask01 && createVSTask02 && createPSTask02).then([this, rootSignature, deviceResources, tempStruct01, tempStruct02]() {


				static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
				state.InputLayout = { inputLayout, _countof(inputLayout) };
				state.pRootSignature = rootSignature.Get();
				state.VS = CD3DX12_SHADER_BYTECODE(&tempStruct01->m_vertexShader[0], tempStruct01->m_vertexShader.size());
				state.PS = CD3DX12_SHADER_BYTECODE(&tempStruct01->m_pixelShader[0], tempStruct01->m_pixelShader.size());
				state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				state.SampleMask = UINT_MAX;
				state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state.NumRenderTargets = 1;
				state.RTVFormats[0] = deviceResources->GetBackBufferFormat();
				state.DSVFormat = deviceResources->GetDepthBufferFormat();
				state.SampleDesc.Count = 1;

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
				state.BlendState.RenderTarget[0] = transparencyBlendDesc;

				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&tempStruct01->m_pipelineState)));

				// 创建管道状态之后可以删除着色器数据。
				tempStruct01->m_vertexShader.clear();
				tempStruct01->m_pixelShader.clear();

			

				static const D3D12_INPUT_ELEMENT_DESC inputLayout02[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state02 = {};
				state02.InputLayout = { inputLayout02, _countof(inputLayout02) };
				state02.pRootSignature = rootSignature.Get();
				state02.VS = CD3DX12_SHADER_BYTECODE(&tempStruct02->m_vertexShader[0], tempStruct02->m_vertexShader.size());
				state02.PS = CD3DX12_SHADER_BYTECODE(&tempStruct02->m_pixelShader[0], tempStruct02->m_pixelShader.size());
				state02.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state02.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state02.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				state02.SampleMask = UINT_MAX;
				state02.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state02.NumRenderTargets = 1;
				state02.RTVFormats[0] = deviceResources->GetBackBufferFormat();
				state02.DSVFormat = deviceResources->GetDepthBufferFormat();
				state02.SampleDesc.Count = 1;



				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state02, IID_PPV_ARGS(&tempStruct02->m_pipelineState)));

				// 创建管道状态之后可以删除着色器数据。
				tempStruct02->m_vertexShader.clear();
				tempStruct02->m_pixelShader.clear();				

			});
#pragma endregion

			createPipelineStateTask.then([this, deviceResources, callBackFun,  tempStruct01, tempStruct02]()
			{
				DX::ThrowIfFailed(deviceResources->GetD3DDevice()->CreateCommandList(
					0, D3D12_COMMAND_LIST_TYPE_DIRECT, deviceResources->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_commandList)));
				NAME_D3D12_OBJECT(m_commandList);
			
				m_pipelineStateMap.insert(pair<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>(L"SampleHLSLShader", tempStruct01));

				m_pipelineStateMap.insert(pair<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>(L"PlaneHLSLShader", tempStruct02));

				if (callBackFun != nullptr)
				{
					callBackFun(true);
				}
			});

	}
	//-----------------------------------

	//void PipelineStateManager::LoadShaderAsync(shared_ptr<PipelineStateInformationStruct> tempStruct01, shared_ptr<PipelineStateInformationStruct> tempStruct02)
	//{
	//	// 通过异步方式加载着色器。
	//	auto createVSTask01 = DX::ReadDataAsync(L"SampleVertexShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
	//		tempStruct01->m_vertexShader = fileData;
	//	});

	//	auto createPSTask01 = DX::ReadDataAsync(L"SamplePixelShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
	//		tempStruct01->m_pixelShader = fileData;
	//	});

	//	auto createVSTask02 = DX::ReadDataAsync(L"PlaneVertexShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
	//		tempStruct01->m_vertexShader = fileData;
	//	});

	//	auto createPSTask02 = DX::ReadDataAsync(L"PlanePixelShader.cso").then([this, tempStruct01](std::vector<byte>& fileData) {
	//		tempStruct01->m_pixelShader = fileData;
	//	});
	//}
	//------
}
