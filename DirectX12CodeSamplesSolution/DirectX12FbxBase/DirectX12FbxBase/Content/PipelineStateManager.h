#pragma once


#include <map>  
#include <string> 
#include <functional>
#include "..\Common\DeviceResources.h"
using namespace std;


namespace DirectX12FbxBase
{
	class PipelineStateManager
	{
	private:
		struct PipelineStateInformationStruct
		{
			//管线状态对象
			Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pipelineState;
			//绑定到管线状态的顶点着色
			std::vector<byte>									m_vertexShader;
			//绑定到管线状态的像素着色
			std::vector<byte>									m_pixelShader;
		};
		//存放管线状态的map
		map<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>			m_pipelineStateMap;	
		//绑定到管线状态的渲染命令列表
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	public:
		PipelineStateManager();

		//创建管线状态对象、加载绑定着色器、创建命令列表02
		void CreateGraphicsPipelineState(
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			std::shared_ptr<DX::DeviceResources> deviceResources,
			std::function<void(HRESULT res)> callBackFun);

		//根据名字、获得对应的管线状态
		inline Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineStageByName(LPCWSTR pipelineName)
		{
			return m_pipelineStateMap[pipelineName]->m_pipelineState;			
		}

		//得到渲染命令列表
		inline Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetGraphicsCommandList()
		{		
			return m_commandList;
		}

		//void LoadShaderAsync(shared_ptr<PipelineStateInformationStruct> tempStruct01, shared_ptr<PipelineStateInformationStruct> tempStruct02);

	//	void CreateGraphicsPipelineState(
	//		LPCWSTR pipelineName, 
	//		std::vector<byte>	vertexShader,	
	//		std::vector<byte>	pixelShader,	
	//		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
	//		std::shared_ptr<DX::DeviceResources> deviceResources);

	};
}

