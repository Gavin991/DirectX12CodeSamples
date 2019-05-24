#pragma once


#include <map>  
#include <string> 
#include <functional>
#include "..\Common\DeviceResources.h"
using namespace std;


namespace DirectX12XAMLXAudio2
{
	class PipelineStateManager
	{
	private:
		struct PipelineStateInformationStruct
		{
			//����״̬����
			Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pipelineState;
			//�󶨵�����״̬�Ķ�����ɫ
			std::vector<byte>									m_vertexShader;
			//�󶨵�����״̬��������ɫ
			std::vector<byte>									m_pixelShader;
		};
		//��Ź���״̬��map
		map<LPCWSTR, shared_ptr<PipelineStateInformationStruct>>			m_pipelineStateMap;	
		//�󶨵�����״̬����Ⱦ�����б�
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	public:
		PipelineStateManager();

		//��������״̬���󡢼��ذ���ɫ�������������б�02
		void CreateGraphicsPipelineState(
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			std::shared_ptr<DX::DeviceResources> deviceResources,
			std::function<void(bool)> callBackFun);	

		//�������֡���ö�Ӧ�Ĺ���״̬
		inline Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineStageByName(LPCWSTR pipelineName)
		{
			return m_pipelineStateMap[pipelineName]->m_pipelineState;			
		}

		//�õ���Ⱦ�����б�
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

