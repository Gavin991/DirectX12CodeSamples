#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"

// 在屏幕上呈现 Direct3D 内容。用于管理应用程序资源、更新应用程序状态和呈现帧的方法。
namespace DirectX12FbxBase
{
	class DirectX12FbxMain
	{
	public:
		DirectX12FbxMain();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void StartTracking() { m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// TODO: 替换为你自己的内容呈现器。
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// 渲染循环计时器。
		DX::StepTimer m_timer;

		// 跟踪当前输入指针位置
		float m_pointerLocationX;
	};
}