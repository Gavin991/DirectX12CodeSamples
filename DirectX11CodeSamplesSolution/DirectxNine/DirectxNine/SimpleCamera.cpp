#include "SimpleCamera.h"

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
	XMFLOAT3 eyePosition,    // the position of the camera
	XMFLOAT3 lookPosition,   // the point the camera should look at
	XMFLOAT3 up              // the durection vector for up
	)
{		
		XMVECTOR eye = XMLoadFloat3(&eyePosition);
		XMVECTOR look = XMLoadFloat3(&lookPosition);
		XMVECTOR upup = XMLoadFloat3(&up);
		m_view = XMMatrixLookAtLH(eye, look, upup);
}

// this method updates the projection matrix based on new parameters
void SimpleCamera::SetProjectionParameters(
	 float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
	 float aspectRatio,         // the aspect ratio of the projection (width / height)
	 float nearPlane,           // depth to map to 0
	 float farPlane             // depth to map to 1
	)
{
	m_projection = XMMatrixPerspectiveFovLH(minimumFieldOfView, aspectRatio, nearPlane, farPlane);
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