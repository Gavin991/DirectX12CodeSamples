#pragma once

namespace DirectX12XAMLPostProcessGray
{
	class GrayGilter
	{
	public:

		GrayGilter();

		//窗口的高宽要与灰度的输入纹理的高宽要一致
		GrayGilter(ID3D12Device* device,UINT width, UINT height,DXGI_FORMAT format);
		ID3D12Resource* Output();

		void BuildDescriptorsForGray(
			CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
			CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
			UINT descriptorSize);

		void OnResize(UINT newWidth, UINT newHeight);

		//灰度计算
		void GrayBehavior(
			ID3D12GraphicsCommandList* cmdList,
			ID3D12RootSignature* rootSig,
			ID3D12PipelineState* grayPSO,		
			ID3D12Resource* input);

		
	private:
		//创建着色器只有视图和无序访问视图
		void BuildSrvAndUav();
		void BuildResources();

		ID3D12Device* md3dDevice = nullptr;

		UINT mWidth = 0;
		UINT mHeight = 0;
		DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		CD3DX12_CPU_DESCRIPTOR_HANDLE mGrayCpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE mGrayCpuUav;

		CD3DX12_GPU_DESCRIPTOR_HANDLE mGrayGpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE mGrayGpuUav;
	
		Microsoft::WRL::ComPtr<ID3D12Resource> mGrayMap = nullptr;

	};

}