#pragma once
using namespace DirectX;

namespace DirectX12XamlPNTriangles
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

	public:
		void GetViewMatrix(XMMATRIX  *viewMatrix);
		void GetProjectionMatrix(XMMATRIX  *projectionMatrix);

		// 根据相机的三个属性更新视觉矩阵
		void SetViewParameters(
			XMVECTOR eyePosition,    // 相机的位置
			XMVECTOR lookPosition,   // 相机观察的位置
			XMVECTOR up              // 向上的向量
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

	};
}

