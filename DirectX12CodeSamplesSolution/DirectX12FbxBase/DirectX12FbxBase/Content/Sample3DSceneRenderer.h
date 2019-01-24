#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "ConstantBufferViewHeapManager.h"
#include "CustomSphere.h"
#include "TextureManager.h"
#include "ConstantPixelShaderBufferInformation.h"
#include "CustomCube.h"
#include "PipelineStateManager.h"
#include "CustomPlane.h"
#include "SimpleCamera.h"
#include "FbxEntity3D.h"
#include "FbxScene3D.h"
#include <tchar.h> 
namespace DirectX12FbxBase
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

		void OnHandleDBELoadBack(HRESULT res);
		void OnHandleCallBack();
		void SetSphereRotateEnable() 
		{ 
			
		 }
		void HandleLightPosChanged(int dir,float x) {
			
		}
		void CubeMappingEnable()
		{
			m_constantPSBuffer.CubeMappingEnable();
		}
		void NormalMappingEnable()
		{
			m_constantPSBuffer.NormalMappingEnable();
		}
		void ParallaxMappingEnable()
		{
			m_constantPSBuffer.ParallaxMappingEnable();
		}

		void HandleCameraXChanged(float x) { m_fCameraX = x;  }//HandleCameraYChanged
		void HandleCameraYChanged(float y) { m_fCameraY = y; }//HandleCameraYChanged

		bool GetIsLoadingComplete() {			return m_loadingComplete;		}
	private:
		void LoadState();
		void Rotate(float radians);

	private:	
		//相机X轴移动量
		float m_fCameraX = 0.0f;
		float m_fCameraY = 0.0f;
		// 常量缓冲区大小必须都是 256 字节的整数倍。
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// 缓存的设备资源指针。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_rootSignature;		

		ConstantBufferViewHeapManager m_cbvHeapManager;

		//CustomSphere m_customSphere;
		CustomCube m_CustomCube;
		CustomPlane m_customPlane;
	
		D3D12_RECT											m_scissorRect;

		std::vector<byte>									m_vertexShader;
		std::vector<byte>									m_pixelShader;


		// 用于渲染循环的变量。
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;
		// 用于保存鼠标左键的偏移量。
		float   m_positionX;


		//Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
		TextureManager m_textureManager;

		//// 像素着色器常量缓存，它也必须是256字节对其的
		//// Constant buffers must be 256-byte aligned.
		ConstantPixelShaderBufferInformation m_constantPSBuffer;

		PipelineStateManager m_pipelineStateManager;

		SimpleCamera             g_SimpleCamera;

		
		int m_standby;

		//FbxEntity3D m_fbxEntity3D;
		FbxManager* lSdkManager = NULL;
		FbxScene* lScene = NULL;
		FbxScene3D m_scene3d;
	public:
		void FbxChildNode(FbxNode* pNode);


		//void PrintNode(FbxNode* pNode) {
		//	PrintTabs();
		//	const char* nodeName = pNode->GetName();
		//	FbxDouble3 translation = pNode->LclTranslation.Get();
		//	FbxDouble3 rotation = pNode->LclRotation.Get();
		//	FbxDouble3 scaling = pNode->LclScaling.Get();

		//	// Print the contents of the node.
		//	//printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
		//	//	nodeName,
		//	//	translation[0], translation[1], translation[2],
		//	//	rotation[0], rotation[1], rotation[2],
		//	//	scaling[0], scaling[1], scaling[2]
		//	//);

		//	TCHAR  sOut[1024];
		//	_stprintf_s(sOut, L"<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n", nodeName,
		//		translation[0], translation[1], translation[2],
		//		rotation[0], rotation[1], rotation[2],
		//		scaling[0], scaling[1], scaling[2]);			
		//	OutputDebugString(sOut);
		//	numTabs++;

		//	// Print the node's attributes.
		//	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		//		PrintAttribute(pNode->GetNodeAttributeByIndex(i));

		//	// Recursively print the children.
		//	for (int j = 0; j < pNode->GetChildCount(); j++)
		//		PrintNode(pNode->GetChild(j));

		//	numTabs--;
		//	PrintTabs();
		//	//printf("</node>\n");
		//	OutputDebugString(L"</node>\n");
		//}

		///**
		//* Print an attribute.
		//*/
		//void PrintAttribute(FbxNodeAttribute* pAttribute) {
		//	if (!pAttribute) return;

		//	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		//	FbxString attrName = pAttribute->GetName();
		//	PrintTabs();

		//	TCHAR*   sOutTypeName;
		//	FbxAnsiToWC(typeName.Buffer(), sOutTypeName);
		//	TCHAR*   sOutAttrName;
		//	FbxAnsiToWC(attrName.Buffer(), sOutAttrName);
		//	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		//	//printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
		//	TCHAR  sOut[512];
		//	_stprintf_s(sOut, L"<attribute type='%s' name='%s'/>\n", sOutTypeName, sOutAttrName);
		//	OutputDebugString(sOut);
		//}

		///**
		//* Return a string-based representation based on the attribute type.
		//*/
		//FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
		//	switch (type) {
		//	case FbxNodeAttribute::eUnknown: return "unidentified";
		//	case FbxNodeAttribute::eNull: return "null";
		//	case FbxNodeAttribute::eMarker: return "marker";
		//	case FbxNodeAttribute::eSkeleton: return "skeleton";
		//	case FbxNodeAttribute::eMesh: return "mesh";
		//	case FbxNodeAttribute::eNurbs: return "nurbs";
		//	case FbxNodeAttribute::ePatch: return "patch";
		//	case FbxNodeAttribute::eCamera: return "camera";
		//	case FbxNodeAttribute::eCameraStereo: return "stereo";
		//	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		//	case FbxNodeAttribute::eLight: return "light";
		//	case FbxNodeAttribute::eOpticalReference: return "optical reference";
		//	case FbxNodeAttribute::eOpticalMarker: return "marker";
		//	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		//	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		//	case FbxNodeAttribute::eBoundary: return "boundary";
		//	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		//	case FbxNodeAttribute::eShape: return "shape";
		//	case FbxNodeAttribute::eLODGroup: return "lodgroup";
		//	case FbxNodeAttribute::eSubDiv: return "subdiv";
		//	default: return "unknown";
		//	}
		//}

		///**
		//* Print the required number of tabs.
		//*/
		//void PrintTabs() {
		//	for (int i = 0; i < numTabs; i++)
		//		//printf("\t");
		//		OutputDebugString(L"\t");
		//}

		///* Tab character ("\t") counter */
		//int numTabs = 0;
	
	};
}

