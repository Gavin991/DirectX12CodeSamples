#pragma once
using namespace DirectX;

namespace DirectX12XAMLDisplacementMap
{

	class SimpleCamera
	{
	public:
		SimpleCamera();
		~SimpleCamera();

	private:
		XMMATRIX  m_view;        // 视觉矩阵		
		XMMATRIX  m_projection;  // 投影矩阵
		bool m_orthogonal;

		XMVECTORF32 m_eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		XMVECTORF32 m_at = { 0.0f, -0.1f, 0.0f, 0.0f };
		XMVECTORF32 m_up = { 0.0f, 1.0f, 0.0f, 0.0f };

	public:
		void GetViewMatrix(XMMATRIX  *viewMatrix);
		void GetProjectionMatrix(XMMATRIX  *projectionMatrix);

		// 根据相机的三个属性更新视觉矩阵
		void SetViewParameters(
			XMVECTORF32 eyePosition,    // 相机的位置
			XMVECTORF32 lookPosition,   // 相机观察的位置
			XMVECTORF32 up              // 向上的向量
		);

		// 根据视锥体的属性更新投影矩阵（透视投影）
		void SetProjectionParameters(
			float minimumFieldOfView,  // 相机的视野角度
			float aspectRatio,         // 投影平面的纵横比
			float nearPlane,           // 近裁剪面
			float farPlane             // 远裁剪面
		);
		//正交投影
		void SetOrthographicParameters(
			float viewWidth,   // 投影平面的宽
			float viewHeight,  // 投影平面的高  
			float nearPlane,   // 近裁剪面   
			float farPlane     // 远裁剪面        
		);

		void UpdateCamera();
		
		void SetCameraPosition(XMVECTORF32 eyePosition);

	};
}

