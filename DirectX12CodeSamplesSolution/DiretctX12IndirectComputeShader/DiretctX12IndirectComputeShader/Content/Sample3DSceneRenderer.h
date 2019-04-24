#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace DiretctX12IndirectComputeShader
{
	// 此示例呈现器实例化一个基本渲染管道。
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Sample3DSceneRenderer();
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void SaveState();

		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }

	private:
		void LoadState();
		void Rotate(float radians);

	private:
		// Constant buffer definition.
		struct SceneConstantBuffer
		{
			DirectX::XMFLOAT4 velocity;
			DirectX::XMFLOAT4 offset;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT4X4 projection;

			// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
			// to be array-indexed.
			float padding[36];
		};

		// 常量缓冲区大小必须都是 256 字节的整数倍。
		//static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		static const UINT c_alignedConstantBufferSize = (sizeof(SceneConstantBuffer) + 255) & ~255;

		// 缓存的设备资源指针。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// 立体几何的 Direct3D 资源。
		//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		//Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_rootSignature;
		//Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pipelineState;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;

		////ModelViewProjectionConstantBuffer					m_constantBufferData;
		//UINT8*												m_mappedConstantBuffer;
		//UINT												m_cbvDescriptorSize;
		D3D12_RECT											m_scissorRect;
		//std::vector<byte>									m_vertexShader;
		//std::vector<byte>									m_pixelShader;
		//D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		//D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		// 用于渲染循环的变量。
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;


		//----------------------

		//static const UINT FrameCount = 3;
		static const UINT SquareCount = 1024;
		static const UINT SquareResourceCount = SquareCount * DX::c_frameCount;
		static const UINT CommandSizePerFrame;				// The size of the indirect commands to draw all of the color squares in a single frame.
		static const UINT CommandBufferCounterOffset;		// The offset of the UAV counter in the processed command buffer.
		static const UINT ComputeThreadBlockSize = 128;		// Should match the value in compute.hlsl.
		static const float SquareHalfWidth ;				// The x and y offsets used by the square vertices.
		static const float SquareDepth;					// The z offset used by the square vertices.
		static const float CullingCutoff;					// The +/- x offset of the clipping planes in homogenous space [-1,1].

															// Vertex definition.
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 uv;
		};



		// Root constants for the compute shader.
		struct CSRootConstants
		{
			float xOffset;
			float zOffset;
			float cullOffset;
			float commandCount;
		};

		// Data structure to match the command signature used for ExecuteIndirect.
		struct IndirectCommand
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbv;
			D3D12_DRAW_ARGUMENTS drawArguments;
		};

		// Graphics root signature parameter offsets.
		enum GraphicsRootParameters
		{
			Cbv,
			Tex,
			GraphicsRootParametersCount
		};

		// Compute root signature parameter offsets.
		enum ComputeRootParameters
		{
			SrvUavTable,
			RootConstants,			// Root constants that give the shader information about the square vertices and culling planes.
			ComputeRootParametersCount
		};

		// CBV/SRV/UAV desciptor heap offsets.
		enum HeapOffsets
		{
			CbvSrvOffset = 0,													// SRV that points to the constant buffers used by the rendering thread.
			CommandsOffset = CbvSrvOffset + 1,									// SRV that points to all of the indirect commands.
			ProcessedCommandsOffset = CommandsOffset + 1,						// UAV that records the commands we actually want to execute.
			CbvSrvUavDescriptorCountPerFrame = ProcessedCommandsOffset + 1		// 2 SRVs + 1 UAV for the compute shader.
		};

		// Each square gets its own constant buffer per frame.
		std::vector<SceneConstantBuffer> m_constantBufferData;
		UINT8* m_pCbvDataBegin;

		CSRootConstants m_csRootConstants;	// Constants for the compute shader.//.....computerShader
		bool m_enableCulling;				// Toggle whether the compute shader pre-processes the indirect commands. /.....computerShader
		D3D12_RECT m_cullingScissorRect; // .....computerShader
		//-------------------------------------------
		//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];//在DeviceResources类总

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_computeCommandAllocators[DX::c_frameCount];//.....computerShader

		//Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;//在DeviceResources类总

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_computeCommandQueue;//.....computerShader

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_computeRootSignature;//.....computerShader


		Microsoft::WRL::ComPtr<ID3D12CommandSignature> m_commandSignature;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap; //在DeviceResources类总
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap; //在DeviceResources类总

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
		//UINT m_rtvDescriptorSize;//在DeviceResources类总
		UINT m_cbvSrvUavDescriptorSize;
		//UINT m_frameIndex;//在DeviceResources类总



		// Asset objects.
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_computeState;//.....computerShader

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_computeCommandList;//.....computerShader

		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;//在DeviceResources类总

		Microsoft::WRL::ComPtr<ID3D12Resource> m_commandBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_processedCommandBuffers[DX::c_frameCount];//.....computerShader
		Microsoft::WRL::ComPtr<ID3D12Resource> m_processedCommandBufferCounterReset;//.....computerShader

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_computeFence;//.....computerShader

		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

		// We pack the UAV counter into the same buffer as the commands rather than create
		// a separate 64K resource/heap for it. The counter must be aligned on 4K boundaries,
		// so we pad the command buffer (if necessary) such that the counter will be placed
		// at a valid location in the buffer.
		static inline UINT AlignForUavCounter(UINT bufferSize)
		{
			const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
			return (bufferSize + (alignment - 1)) & ~(alignment - 1);
		}

		inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
		{
			if (path == nullptr)
			{
				throw std::exception();
			}

			DWORD size = GetModuleFileName(nullptr, path, pathSize);
			if (size == 0 || size == pathSize)
			{
				// Method failed or path was truncated.
				throw std::exception();
			}

			WCHAR* lastSlash = wcsrchr(path, L'\\');
			if (lastSlash)
			{
				*(lastSlash + 1) = L'\0';
			}
		}

		// Get a random float value between min and max.
		inline float GetRandomFloat(float min, float max)
		{
			float scale = static_cast<float>(rand()) / RAND_MAX;
			float range = max - min;
			return scale * range + min;
		}


		////------------------------------------------------------------------------------------------------
		//// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
		//// To help enable root signature 1.1 features when they are available and not require maintaining
		//// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
		//// 1.1 is not supported.
		//inline HRESULT D3DX12SerializeVersionedRootSignature(
		//	_In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
		//	D3D_ROOT_SIGNATURE_VERSION MaxVersion,
		//	_Outptr_ ID3DBlob** ppBlob,
		//	_Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
		//{
		//	if (ppErrorBlob != NULL)
		//	{
		//		*ppErrorBlob = NULL;
		//	}

		//	switch (MaxVersion)
		//	{
		//	case D3D_ROOT_SIGNATURE_VERSION_1_0:
		//		switch (pRootSignatureDesc->Version)
		//		{
		//		case D3D_ROOT_SIGNATURE_VERSION_1_0:
		//			return D3D12SerializeRootSignature(&pRootSignatureDesc->Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);

		//		case D3D_ROOT_SIGNATURE_VERSION_1_1:
		//		{
		//			HRESULT hr = S_OK;
		//			const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

		//			const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
		//			void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : NULL;
		//			if (ParametersSize > 0 && pParameters == NULL)
		//			{
		//				hr = E_OUTOFMEMORY;
		//			}
		//			D3D12_ROOT_PARAMETER* pParameters_1_0 = reinterpret_cast<D3D12_ROOT_PARAMETER*>(pParameters);

		//			if (SUCCEEDED(hr))
		//			{
		//				for (UINT n = 0; n < desc_1_1.NumParameters; n++)
		//				{
		//					__analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
		//					pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
		//					pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

		//					switch (desc_1_1.pParameters[n].ParameterType)
		//					{
		//					case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		//						pParameters_1_0[n].Constants.Num32BitValues = desc_1_1.pParameters[n].Constants.Num32BitValues;
		//						pParameters_1_0[n].Constants.RegisterSpace = desc_1_1.pParameters[n].Constants.RegisterSpace;
		//						pParameters_1_0[n].Constants.ShaderRegister = desc_1_1.pParameters[n].Constants.ShaderRegister;
		//						break;

		//					case D3D12_ROOT_PARAMETER_TYPE_CBV:
		//					case D3D12_ROOT_PARAMETER_TYPE_SRV:
		//					case D3D12_ROOT_PARAMETER_TYPE_UAV:
		//						pParameters_1_0[n].Descriptor.RegisterSpace = desc_1_1.pParameters[n].Descriptor.RegisterSpace;
		//						pParameters_1_0[n].Descriptor.ShaderRegister = desc_1_1.pParameters[n].Descriptor.ShaderRegister;
		//						break;

		//					case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		//						const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 = desc_1_1.pParameters[n].DescriptorTable;

		//						const SIZE_T DescriptorRangesSize = sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
		//						void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr)) ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize) : NULL;
		//						if (DescriptorRangesSize > 0 && pDescriptorRanges == NULL)
		//						{
		//							hr = E_OUTOFMEMORY;
		//						}
		//						D3D12_DESCRIPTOR_RANGE* pDescriptorRanges_1_0 = reinterpret_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

		//						if (SUCCEEDED(hr))
		//						{
		//							for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++)
		//							{
		//								__analysis_assume(DescriptorRangesSize == sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
		//								pDescriptorRanges_1_0[x].BaseShaderRegister = table_1_1.pDescriptorRanges[x].BaseShaderRegister;
		//								pDescriptorRanges_1_0[x].NumDescriptors = table_1_1.pDescriptorRanges[x].NumDescriptors;
		//								pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart = table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
		//								pDescriptorRanges_1_0[x].RangeType = table_1_1.pDescriptorRanges[x].RangeType;
		//								pDescriptorRanges_1_0[x].RegisterSpace = table_1_1.pDescriptorRanges[x].RegisterSpace;
		//							}
		//						}

		//						D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
		//						table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
		//						table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
		//					}
		//				}
		//			}

		//			if (SUCCEEDED(hr))
		//			{
		//				CD3DX12_ROOT_SIGNATURE_DESC desc_1_0(desc_1_1.NumParameters, pParameters_1_0, desc_1_1.NumStaticSamplers, desc_1_1.pStaticSamplers, desc_1_1.Flags);
		//				hr = D3D12SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
		//			}

		//			if (pParameters)
		//			{
		//				for (UINT n = 0; n < desc_1_1.NumParameters; n++)
		//				{
		//					if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		//					{
		//						HeapFree(GetProcessHeap(), 0, reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(pParameters_1_0[n].DescriptorTable.pDescriptorRanges)));
		//					}
		//				}
		//				HeapFree(GetProcessHeap(), 0, pParameters);
		//			}
		//			return hr;
		//		}
		//		}
		//		break;

		//	case D3D_ROOT_SIGNATURE_VERSION_1_1:
		//		return D3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
		//	}

		//	return E_INVALIDARG;
		//}

	};
}

