#include "pch.h"
#include "DirectX12FbxMain.h"
#include "Common\DirectXHelper.h"

using namespace DirectX12FbxBase;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;




//加载应用程序时加载并初始化应用程序资产。
DirectX12FbxMain::DirectX12FbxMain() :
	m_pointerLocationX(0.0f)
{
	// TODO: 如果需要默认的可变时间步长模式之外的其他模式，请更改计时器设置。
	// 例如，对于 60 FPS 固定时间步长更新逻辑，请调用:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

// 创建并初始化呈现器。
void DirectX12FbxMain::CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	//TODO: 将此替换为应用程序内容的初始化。
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(deviceResources));

	OnWindowSizeChanged();
}

void DirectX12FbxMain::StartRenderLoop(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	// 如果动画呈现循环已在运行，则勿启动其他线程。
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// 创建一个将在后台线程上运行的任务。
	auto workItemHandler = ref new WorkItemHandler([this, deviceResources](IAsyncAction ^ action)
	{
		// 计算更新的帧并且在每个场消隐期呈现一次。
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			if (deviceResources->IsDeviceRemoved())
			{				
				// 当新设备可用时将创建一个的新线程，退出渲染线程。
				break;
			}

			auto commandQueue = deviceResources->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				Update();
			}
			PIXEndEvent(commandQueue);

			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (Render())
				{
					deviceResources->Present();
				}
			}
			PIXEndEvent(commandQueue);
		}
	});

	//在高优先级的专用后台线程上运行任务。
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DirectX12FbxMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

//  每帧更新一次应用程序状态。
void DirectX12FbxMain::Update()
{
	ProcessInput();

	//  每帧更新一次应用程序状态。
	m_timer.Tick([&]()
	{
		// TODO: 将此替换为应用程序内容的更新函数。
		m_sceneRenderer->Update(m_timer);
	});
}

//在更新游戏状态之前处理所有用户输入
void DirectX12FbxMain::ProcessInput()
{
	//  TODO: 按帧输入处理在此处添加。
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// 根据当前应用程序状态呈现当前帧。
// 如果帧已呈现并且已准备好显示，则返回 true。
bool DirectX12FbxMain::Render()
{
	// 在首次更新前，请勿尝试呈现任何内容。
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	// 呈现场景对象。
	// TODO: 将此替换为应用程序内容的渲染函数。
	return m_sceneRenderer->Render();
}

//在窗口大小更改(例如，设备方向更改)时更新应用程序状态
void DirectX12FbxMain::OnWindowSizeChanged()
{
	//  TODO: 将此替换为应用程序内容的与大小相关的初始化。
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// 向应用通知它将挂起。
void DirectX12FbxMain::OnSuspending()
{
	// TODO: 将此内容替换为应用的挂起逻辑。

	// 进程周期管理可能会随时终止挂起的应用，因此
	// 最好保存使应用可以在中断位置重新启动的任何状态。

	m_sceneRenderer->SaveState();

	// 如果应用程序使用易于重新创建的视频内存分配，
	// 请考虑释放该内存以使它可供其他应用程序使用。
}

// 向应用通知它将不再挂起。
void DirectX12FbxMain::OnResuming()
{
	// TODO: 将此内容替换为应用的恢复逻辑。
}

// 通知呈现器，需要释放设备资源。
void DirectX12FbxMain::OnDeviceRemoved()
{
	// TODO: 保存任何所需的应用程序或呈现器状态，然后释放不再
	// 有效的呈现器及其资源。
	m_sceneRenderer->SaveState();
	m_sceneRenderer = nullptr;
}
