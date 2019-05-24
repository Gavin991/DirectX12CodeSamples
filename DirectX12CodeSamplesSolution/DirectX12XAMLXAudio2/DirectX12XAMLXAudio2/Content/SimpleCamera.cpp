#include "pch.h"
#include "SimpleCamera.h"


using namespace DirectX12XAMLXAudio2;

//#define PI_F 3.1415927f

SimpleCamera::SimpleCamera()
{
}


SimpleCamera::~SimpleCamera()
{
}

void SimpleCamera::GetViewMatrix(XMMATRIX *viewMatrix)
{
	*viewMatrix = m_view;
}

void SimpleCamera::GetProjectionMatrix(XMMATRIX *projectionMatrix)
{
	*projectionMatrix = m_projection;
}

// this method updates the view matrix based on new position and focus coordinates
void SimpleCamera::SetViewParameters(
	XMVECTORF32 eyePosition,    // the position of the camera
	XMVECTORF32 lookPosition,   // the point the camera should look at
	XMVECTORF32 up              // the durection vector for up XMVECTOR
	)
{
	m_eye = eyePosition;
	m_at = lookPosition;
	m_up = up;
	m_view = XMMatrixTranspose(XMMatrixLookAtRH(eyePosition, lookPosition, up));
}


// this method updates the projection matrix based on new parameters
void SimpleCamera::SetProjectionParameters(
	 float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
	 float aspectRatio,         // the aspect ratio of the projection (width / height)
	 float nearPlane,           // depth to map to 0
	 float farPlane             // depth to map to 1
	)
{
	m_projection = XMMatrixPerspectiveFovRH(minimumFieldOfView, aspectRatio, nearPlane, farPlane);
	m_orthogonal = false;
}


void SimpleCamera::SetOrthographicParameters(
	float viewWidth,		
	float viewHeight,         
	float nearPlane,           
	float farPlane            
	)
{	
	m_projection = XMMatrixOrthographicLH(viewHeight, viewHeight, nearPlane, farPlane);//40,30, -100, 100
	m_orthogonal = true;
}

void SimpleCamera::UpdateCamera()
{
	m_view = XMMatrixTranspose(XMMatrixLookAtRH(m_eye, m_at, m_up));
}

void SimpleCamera::SetCameraPosition(XMVECTORF32 eyePosition)
{
	m_eye = eyePosition;
}