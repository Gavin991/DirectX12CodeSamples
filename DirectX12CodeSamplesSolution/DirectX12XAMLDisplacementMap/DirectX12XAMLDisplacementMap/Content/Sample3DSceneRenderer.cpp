#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include <ppltasks.h>
#include <synchapi.h>

#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"

using namespace DirectX12XAMLDisplacementMap;

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Storage;

// 应用程序状态映射中的索引。
Platform::String^ AngleKey = "Angle";
Platform::String^ TrackingKey = "Tracking";

//void CreateSphereVector(float radius, int slice, int stack, std::vector<VertexPositionColor> &VertexArray, std::vector<unsigned short> &indexArray);
//int nIndices;

// 从文件中加载顶点和像素着色器，然后实例化立方体几何图形。
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_radiansPerSecond(XM_PIDIV4),	// 每秒旋转 45 度
	m_angle(0),
	m_tracking(false),
	m_positionX(0),
	m_deviceResources(deviceResources)
{
	LoadState();
	//ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

Sample3DSceneRenderer::~Sample3DSceneRenderer()
{
	//m_constantBuffer->Unmap(0, nullptr);
	//m_mappedConstantBuffer = nullptr;
	m_customSphere.ClearConstantBuffer();
	m_constantPSBuffer.ClearConstantBuffer();
	m_CustomCube.ClearConstantBuffer();
	m_customPlane.ClearConstantBuffer();
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	auto d3dDevice = m_deviceResources->GetD3DDevice();	


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
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//D3D12_SHADER_VISIBILITY_PIXEL;

		//编号0 顶点缓存
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

		//编号1 第一张纹理缓存
		D3D12_DESCRIPTOR_RANGE range2;
		range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range2.NumDescriptors = 1;
		range2.BaseShaderRegister = 0;
		range2.RegisterSpace = 0;
		range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter2;
		parameter2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter2.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;// D3D12_SHADER_VISIBILITY_PIXEL;
		parameter2.DescriptorTable.NumDescriptorRanges = 1;
		parameter2.DescriptorTable.pDescriptorRanges = &range2;

		//编号2 像素着色器缓存
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

		//编号3 第二张纹理缓存
		D3D12_DESCRIPTOR_RANGE range4;
		range4.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range4.NumDescriptors = 1;
		range4.BaseShaderRegister = 1;
		range4.RegisterSpace = 0;
		range4.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter4;
		parameter4.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter4.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//D3D12_SHADER_VISIBILITY_PIXEL;
		parameter4.DescriptorTable.NumDescriptorRanges = 1;
		parameter4.DescriptorTable.pDescriptorRanges = &range4;

		//编号3 第三张纹理缓存
		D3D12_DESCRIPTOR_RANGE range5;
		range5.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range5.NumDescriptors = 1;
		range5.BaseShaderRegister = 2;
		range5.RegisterSpace = 0;
		range5.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER parameter5;
		parameter5.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter5.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;// D3D12_SHADER_VISIBILITY_PIXEL;
		parameter5.DescriptorTable.NumDescriptorRanges = 1;
		parameter5.DescriptorTable.pDescriptorRanges = &range5;

		//D3D12_ROOT_PARAMETER parameters[3];
		D3D12_ROOT_PARAMETER parameters[5];
		parameters[0] = parameter1;
		parameters[1] = parameter2;
		parameters[2] = parameter3;
		parameters[3] = parameter4;
		parameters[4] = parameter5;

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

	//创建图形管线状态
	auto callBackFun = std::bind(&Sample3DSceneRenderer::OnHandleCallBack, this, std::placeholders::_1);
	m_pipelineStateManager.CreateGraphicsPipelineState(m_rootSignature, m_deviceResources, callBackFun);


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
	//XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
	//	fovAngleY,
	//	aspectRatio,
	//	0.01f,
	//	100.0f
	//	);

	XMMATRIX perspectiveMatrix;
	g_SimpleCamera.SetProjectionParameters(fovAngleY, aspectRatio, 0.01f, 1000.0f);
	g_SimpleCamera.GetProjectionMatrix(&perspectiveMatrix);


	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	//XMStoreFloat4x4(
	//	&m_constantBufferData.projection,
	//	XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	//	);
	XMMATRIX modelMatrix = XMMatrixTranspose(perspectiveMatrix * orientationMatrix);

	// 眼睛位于(0,0.7,1.5)，并沿着 Y 轴使用向上矢量查找点(0,-0.1,0)。
	static const XMVECTORF32 eye = { 0.0f, 0.0f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	//XMMATRIX viewMatrix = XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up));

	XMMATRIX viewMatrix;
	g_SimpleCamera.SetViewParameters(eye, at, up);
	g_SimpleCamera.GetViewMatrix(&viewMatrix);

	m_customSphere.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_CustomCube.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_customPlane.SetModelViewMatrix(modelMatrix, viewMatrix);
	//XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// 每个帧调用一次，旋转立方体，并计算模型和视图矩阵。
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{

	XMMATRIX perspectiveMatrix;
	g_SimpleCamera.GetProjectionMatrix(&perspectiveMatrix);
	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
	XMMATRIX modelMatrix = XMMatrixTranspose(perspectiveMatrix * orientationMatrix);

	
	 XMVECTORF32 eye = { m_fCameraX,m_fCameraY, 1.5f, 0.0f };
	 g_SimpleCamera.SetCameraPosition(eye);

	XMMATRIX viewMatrix;
	g_SimpleCamera.UpdateCamera();
	g_SimpleCamera.GetViewMatrix(&viewMatrix);

	m_customSphere.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_CustomCube.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_customPlane.SetModelViewMatrix(modelMatrix, viewMatrix);

	if (m_loadingComplete)
	{
		if (!m_tracking)
		{
			// 少量旋转立方体。
			float curElapseTime= static_cast<float>(timer.GetElapsedSeconds()) * m_radiansPerSecond;
			m_angle += curElapseTime;

			Rotate(m_angle);

			m_customSphere.UpdateElapsedSeconds(curElapseTime);
		}

		//// 更新常量缓冲区资源。
		//UINT8* destination = m_mappedConstantBuffer + (m_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		//memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

		// Update the ambient color
		//m_constantPSBufferData.ambientColor = XMFLOAT4(0.31f, 0.31f, 0.31f,  1.0f); // light black
		//m_constantPSBufferData.specularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // red directional red color
		//m_constantPSBufferData.lightDirection = XMFLOAT3(-1.0f, -1.0f, -1.0f); // light in the z direction	
		//m_constantPSBufferData.power = 15.0f;
		//memcpy(m_mappedConstantPSBuffer, &m_constantPSBufferData, sizeof(m_constantPSBufferData));
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
	//m_constantBufferData.lightDirction = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);
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
		m_positionX = positionX;
		//float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		//Rotate(radians);
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
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList = m_pipelineStateManager.GetGraphicsCommandList();
	DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineStateManager.GetPipelineStageByName(L"PlaneHLSLShader").Get()));
	m_commandList->SetPipelineState(m_pipelineStateManager.GetPipelineStageByName(L"PlaneHLSLShader").Get());

	PIXBeginEvent(m_commandList.Get(), 0, L"Draw the cube");
	{
		// 设置要由此帧使用的图形根签名和描述符堆。
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		//ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeapManager.GetCbvHeap().Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// 将当前帧的常量缓冲区绑定到管道。
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle3(m_cbvHeapManager.GetGPUDescriptorHandleForHeapStart(), DX::c_frameCount+1+3, m_cbvHeapManager.GetCbvDescriptorSize());
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

		m_constantPSBuffer.Update();
		m_constantPSBuffer.BindPSConstantBuffer(&m_cbvHeapManager, m_commandList, 2);

		DirectX::XMMATRIX curCubeMatrix;

		//使用第一张纹理，渲染第一个木地板
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\CubeMap.dds", 1, m_commandList);
		//curCubeMatrix = XMMatrixScaling(5.0f, 0.05f, 5.0f)*XMMatrixTranslation(0, -0.7f, 0);
		//m_customPlane.UpdatePlaneByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix);
		//m_customPlane.DrawPlane(m_deviceResources->GetCurrentFrameIndex());

		curCubeMatrix = XMMatrixScaling(100.0f, 100.0f, 100.0f);
		m_CustomCube.UpdateCubeByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix);
		m_CustomCube.DrawCube(m_deviceResources->GetCurrentFrameIndex());

		//使用第二个管线状态，渲染球体
		Microsoft::WRL::ComPtr<ID3D12PipelineState> second = m_pipelineStateManager.GetPipelineStageByName(L"SampleHLSLShader");
		m_commandList->SetPipelineState(second.Get());

		//使用第一张纹理，渲染第一个球体
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\brick_wall.jpg", 3, m_commandList);
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\brick_wall_normal.dds", 4, m_commandList);
		DirectX::XMFLOAT3 offsetPos = { 0.7f,0,0 };
		m_customSphere.UpdateSphereByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), m_angle, offsetPos);
		m_customSphere.DrawSphere();

		//使用第二张纹理，渲染第二个球体
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\stone_brown.jpg", 3, m_commandList);
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\stone_brown_normal.dds", 4, m_commandList);
		offsetPos = { -0.7f,0,0 };
		m_customSphere.UpdateSphereByIndex(&m_cbvHeapManager, 1, m_deviceResources->GetCurrentFrameIndex(), m_angle, offsetPos);
		m_customSphere.DrawSphere();

		////使用第三张纹理，渲染第一个立方体
		////m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\CubeMap.dds", 1, m_commandList);	//	pic03.jpg
		//if (m_tracking)
		//{
		//	float radians = XM_2PI * 2.0f * m_positionX  / m_deviceResources->GetOutputSize().Width;		
		//	curCubeMatrix = XMMatrixRotationY(radians)*XMMatrixScaling(0.5f, 0.5f, 0.5f);	//	0.5f, 0.5f, 0.5f	
		//}
		//else
		//{
		//	curCubeMatrix = XMMatrixRotationY(-m_angle*0.5f)*XMMatrixScaling(0.5f, 0.5f, 0.5f);
		//}

		////curCubeMatrix = XMMatrixScaling(10.0f, 10.0f, 10.0f);

		//m_CustomCube.UpdateCubeByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix);

		//m_CustomCube.DrawCube(m_deviceResources->GetCurrentFrameIndex());

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

void Sample3DSceneRenderer::OnHandleCallBack(bool res)
{
	// 创建立方体几何图形资源并上载到 GPU。
	auto createAssetslambda = [this]() {
		auto d3dDevice = m_deviceResources->GetD3DDevice();

		// 创建命令列表。
		//DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList = m_pipelineStateManager.GetGraphicsCommandList();
		NAME_D3D12_OBJECT(m_commandList);

		m_customSphere.Initialize(d3dDevice, m_commandList, 2);
		m_customSphere.CreateSphereVector();
		m_customSphere.UpdateVertexAndIndexSubresources();

		m_CustomCube.Initialize(d3dDevice, m_commandList, 1);
		m_CustomCube.CreateCubeVector();
		m_CustomCube.UpdateVertexAndIndexSubresources();

		m_customPlane.Initialize(d3dDevice, m_commandList, 1);
		m_customPlane.CreatePlaneVector();
		m_customPlane.UpdateVertexAndIndexSubresources();

		// 为常量缓冲区创建描述符堆。
		{
			int custom_geometry_num = m_customSphere.GetSphereNumber() + m_CustomCube.GetCubeNumber()+ m_customPlane.GetPlaneNumber();
			int numDescriptos = DX::c_frameCount*custom_geometry_num + 1 + 1;
			m_cbvHeapManager.Initialize(d3dDevice, numDescriptos);
		}

		m_customSphere.UpLoadVextureConstantBuffer(&m_cbvHeapManager);
		// 创建顶点/索引缓冲区视图。
		m_customSphere.CreateBufferView();

		m_CustomCube.UpLoadVextureConstantBuffer(&m_cbvHeapManager);
		m_CustomCube.CreateBufferView();

		m_customPlane.UpLoadVextureConstantBuffer(&m_cbvHeapManager);
		m_customPlane.CreateBufferView();


#pragma region 加载纹理部分
		m_textureManager.Initialize(d3dDevice);
		m_textureManager.LoadTexture(&m_cbvHeapManager, L"Assets\\brick_wall.jpg", m_commandList);
		m_textureManager.LoadDDsTexture(&m_cbvHeapManager, L"Assets\\brick_wall_normal.dds", m_commandList, D3D12_SRV_DIMENSION_TEXTURE2D);

		m_textureManager.LoadTexture(&m_cbvHeapManager, L"Assets\\stone_brown.jpg", m_commandList);
		m_textureManager.LoadDDsTexture(&m_cbvHeapManager, L"Assets\\stone_brown_normal.dds", m_commandList, D3D12_SRV_DIMENSION_TEXTURE2D);

		m_textureManager.LoadDDsTexture(&m_cbvHeapManager, L"Assets\\CubeMap.dds", m_commandList, D3D12_SRV_DIMENSION_TEXTURECUBE);

#pragma endregion


#pragma region 像素着色器常量缓存 Pixel Shader Constant Buffer
		const UINT c_AlignedConstantPSBufferSize = (sizeof(ConstantPSBuffer) + 255) & ~255;
		m_constantPSBuffer.PixelConstantBuffer(&m_cbvHeapManager, d3dDevice, c_AlignedConstantPSBufferSize);
#pragma endregion

		



		// 关闭命令列表并执行它，以开始将顶点/索引缓冲区复制到 GPU 的默认堆中。
		DX::ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// 等待命令列表完成执行；顶点/索引缓冲区需要在上载资源超出范围之前上载到 GPU。
		m_deviceResources->WaitForGpu();
	};

	auto createAssetsTask = create_task(createAssetslambda);
	createAssetsTask.then([this]() {
		m_loadingComplete = true;
		m_customSphere.RemoveUselessContent();
		m_textureManager.RemoveUselessContent();
		m_CustomCube.RemoveUselessContent();
		m_customPlane.RemoveUselessContent();
	});
}