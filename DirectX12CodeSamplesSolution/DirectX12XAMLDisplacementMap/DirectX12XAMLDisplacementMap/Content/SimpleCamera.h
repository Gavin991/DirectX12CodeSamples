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
		XMMATRIX  m_view;        // �Ӿ�����		
		XMMATRIX  m_projection;  // ͶӰ����
		bool m_orthogonal;

		XMVECTORF32 m_eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		XMVECTORF32 m_at = { 0.0f, -0.1f, 0.0f, 0.0f };
		XMVECTORF32 m_up = { 0.0f, 1.0f, 0.0f, 0.0f };

	public:
		void GetViewMatrix(XMMATRIX  *viewMatrix);
		void GetProjectionMatrix(XMMATRIX  *projectionMatrix);

		// ����������������Ը����Ӿ�����
		void SetViewParameters(
			XMVECTORF32 eyePosition,    // �����λ��
			XMVECTORF32 lookPosition,   // ����۲��λ��
			XMVECTORF32 up              // ���ϵ�����
		);

		// ������׶������Ը���ͶӰ����͸��ͶӰ��
		void SetProjectionParameters(
			float minimumFieldOfView,  // �������Ұ�Ƕ�
			float aspectRatio,         // ͶӰƽ����ݺ��
			float nearPlane,           // ���ü���
			float farPlane             // Զ�ü���
		);
		//����ͶӰ
		void SetOrthographicParameters(
			float viewWidth,   // ͶӰƽ��Ŀ�
			float viewHeight,  // ͶӰƽ��ĸ�  
			float nearPlane,   // ���ü���   
			float farPlane     // Զ�ü���        
		);

		void UpdateCamera();
		
		void SetCameraPosition(XMVECTORF32 eyePosition);

	};
}

