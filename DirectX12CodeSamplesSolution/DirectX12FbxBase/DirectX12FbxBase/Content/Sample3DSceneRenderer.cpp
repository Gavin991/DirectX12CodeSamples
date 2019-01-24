#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include "..\Common\FbxHelper.h"
#include <ppltasks.h>
#include <synchapi.h>
#include "FbxScene3D.h"
#include "..\..\DirectXTex-master\DirectXTex\DirectXTex.h"

using namespace DirectX12FbxBase;

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Storage;


//using namespace Windows::UI::Core;
using namespace Windows::ApplicationModel;

//using namespace Windows::UI::ViewManagement;
//using namespace Windows::Graphics::Display;
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
	
	m_constantPSBuffer.ClearConstantBuffer();
	m_CustomCube.ClearConstantBuffer();
	m_customPlane.ClearConstantBuffer();	
	m_scene3d.ClearFbxScene3DConstantBuffer();

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
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
		parameter2.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
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
		parameter4.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
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
		parameter5.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
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

	m_standby = 0;
	//创建图形管线状态
	auto callBackFun = std::bind(&Sample3DSceneRenderer::OnHandleDBELoadBack, this, std::placeholders::_1);
	m_pipelineStateManager.CreateGraphicsPipelineState(m_rootSignature, m_deviceResources, callBackFun);

	// 加载FBX文件。
	auto LoadFbxlambda = [this]() {
		// Prepare the FBX SDK.
		DX::InitializeSdkObjects(lSdkManager, lScene);
		FbxString lFilePath("Assets\\Tow_Gauss1.fbx");
		DX::LoadScene(lSdkManager, lScene, lFilePath.Buffer());

		

		// Convert mesh, NURBS and patch into triangle mesh
		FbxGeometryConverter lGeomConverter(lSdkManager);
		lGeomConverter.Triangulate(lScene, /*replace*/true);
	};

	auto LoadFbxTask = create_task(LoadFbxlambda);
	LoadFbxTask.then([this]() {
		OnHandleDBELoadBack(S_OK);
	});

	//auto callBackFunForDBE = std::bind(&Sample3DSceneRenderer::OnHandleDBELoadBack, this, std::placeholders::_1);
	//Platform::String^ name = L"Assets\\Banner.xml";//Banner.xmlProp_Barrel banner.xml box.xml Prop_Barrel.xml env_lobby_grassA.xml

	//// Change the following filename to a suitable filename value.
	//auto location = Package::Current->InstalledLocation;
	//Platform::String^ path = Platform::String::Concat(location->Path, "\\");
	//path = Platform::String::Concat(path, "Assets\\Sadface.FBX");

	////Platform::String^ path = "aoeu";
	//std::wstring fooW(path->Begin());
	//std::string fooA(fooW.begin(), fooW.end());
	////const char* charStr = fooA.c_str();

	//const char* lFilename = fooA.c_str();
	//const char* lFilename = "Assets\\Tow_Gauss1.fbx";//./Assets/

	// Initialize the SDK manager. This object handles memory management.
	//FbxManager* lSdkManager = NULL;
	//FbxScene* lScene = NULL;
	//// Prepare the FBX SDK.
	//DX::InitializeSdkObjects(lSdkManager, lScene);

	//FbxString lFilePath("Assets\\Tow_Gauss1.fbx");
	//DX::LoadScene(lSdkManager, lScene, lFilePath.Buffer());
	
	//// Load the scene.
	//////FbxManager* lSdkManager = FbxManager::Create();

	////// Create the IO settings object.
	//////FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	//////lSdkManager->SetIOSettings(ios);

	/////*FbxString lPath = FbxGetApplicationDirectory();
	////lSdkManager->LoadPluginsDirectory(lPath.Buffer());*/

	////// Create an importer using the SDK manager.
	////FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	////// Use the first argument as the filename for the importer.//
	////if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings()))
	////{
	////	printf("Call to FbxImporter::Initialize() failed.\n");
	////	printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
	////	exit(-1);
	////}
	//

	////// Create a new scene so that it can be populated by the imported file.
	//////FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	////// Import the contents of the file into the scene.
	////lImporter->Import(lScene);

	//// Convert mesh, NURBS and patch into triangle mesh
	//FbxGeometryConverter lGeomConverter(lSdkManager);
	//lGeomConverter.Triangulate(lScene, /*replace*/true);


	/*FbxChildNode(lScene->GetRootNode());*/

	// The file is imported, so get rid of the importer.
	//lImporter->Destroy();

	// Destroy the SDK manager and all the other objects it was handling.
	/*lSdkManager->Destroy();*/

}

void  Sample3DSceneRenderer::FbxChildNode(FbxNode* pNode)
{
	const int TRIANGLE_VERTEX_COUNT = 3;

	// Four floats for every position.
	const int VERTEX_STRIDE = 4;
	// Three floats for every normal.
	const int NORMAL_STRIDE = 3;
	// Two floats for every UV.
	const int UV_STRIDE = 2;

	FbxString lString;

	// Print the nodes of the scene and their attributes recursively.
	// Note that we are not printing the root node because it should
	// not contain any attributes.
	// Bake mesh as VBO(vertex buffer object) into GPU.
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	lString = pNode->GetName();
	FbxAMatrix& lLocalTransform = pNode->EvaluateLocalTransform();
	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			bool mHasNormal;
			bool mHasUV;
			bool mAllByControlPoint; // Save data in VBO by control point or by polygon vertex.

			FbxMesh * lMesh = pNode->GetMesh();
			lString=lMesh->GetName();
			if (lMesh && !lMesh->GetUserDataPtr())
			{
				const int lPolygonCount = lMesh->GetPolygonCount();


				lString = " Mesh Polygon Count 三角面 =======>>>>>>:";
				lString += lPolygonCount;
				lString += "\n";
				FBXSDK_printf(lString);

				// Congregate all the data of a mesh to be cached in VBOs.
				// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
				bool mHasNormal = lMesh->GetElementNormalCount() > 0;
				bool mHasUV = lMesh->GetElementUVCount() > 0;
				FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
				FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
				if (mHasNormal)
				{
					lNormalMappingMode = lMesh->GetElementNormal(0)->GetMappingMode();
					if (lNormalMappingMode == FbxGeometryElement::eNone)
					{
						mHasNormal = false;
					}
					if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
					{
						mAllByControlPoint = false;
					}
				}
				if (mHasUV)
				{
					lUVMappingMode = lMesh->GetElementUV(0)->GetMappingMode();
					if (lUVMappingMode == FbxGeometryElement::eNone)
					{
						mHasUV = false;
					}
					if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
					{
						mAllByControlPoint = false;
					}
				}

				// Allocate the array memory, by control point or by polygon vertex.
				int lPolygonVertexCount = lMesh->GetControlPointsCount();
				lString = " lPolygon Vertex Count=======>>>>>>:\n";
				lString += lPolygonVertexCount;
				lString += "\n";
				FBXSDK_printf(lString);

				if (!mAllByControlPoint)
				{
					lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
				}

				float * lVertices = new float[lPolygonVertexCount * VERTEX_STRIDE];
				unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
				float * lNormals = NULL;
				if (mHasNormal)
				{
					lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
				}
				float * lUVs = NULL;
				FbxStringList lUVNames;
				lMesh->GetUVSetNames(lUVNames);
				const char * lUVName = NULL;
				if (mHasUV && lUVNames.GetCount())
				{
					lUVs = new float[lPolygonVertexCount * UV_STRIDE];
					lUVName = lUVNames[0];
				}

				// Populate the array with vertex attribute, if by control point.
				const FbxVector4 * lControlPoints = lMesh->GetControlPoints();
				FbxVector4 lCurrentVertex;
				FbxVector4 lCurrentNormal;
				FbxVector2 lCurrentUV;

				if (mAllByControlPoint)
				{
					const FbxGeometryElementNormal * lNormalElement = NULL;
					const FbxGeometryElementUV * lUVElement = NULL;
					if (mHasNormal)
					{
						lNormalElement = lMesh->GetElementNormal(0);
					}
					if (mHasUV)
					{
						lUVElement = lMesh->GetElementUV(0);
					}
					for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
					{
						// Save the vertex position.
						lCurrentVertex = lControlPoints[lIndex];
						lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
						lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
						lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
						lVertices[lIndex * VERTEX_STRIDE + 3] = 1;

						// Save the normal.
						if (mHasNormal)
						{
							int lNormalIndex = lIndex;
							if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
							{
								lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
							}
							lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
							lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
							lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
							lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
						}

						// Save the UV.
						if (mHasUV)
						{
							int lUVIndex = lIndex;
							if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
							{
								lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
							}
							lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
							lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
							lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
						}
					}

				}//-------------------if (mAllByControlPoint)----------------end

				int lVertexCount = 0;
				for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
				{
					// The material for current face.
					//int lMaterialIndex = 0;						
					for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
					{
						//第几个面的，第几个顶点的索引
						const int lControlPointIndex = lMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

						if (mAllByControlPoint)
						{
							lIndices[lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
						}
						// Populate the array with vertex attribute, if by polygon vertex.
						else
						{
							lIndices[lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

							lCurrentVertex = lControlPoints[lControlPointIndex];
							lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
							lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
							lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
							lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;

							if (mHasNormal)
							{
								lMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
								lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
								lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
								lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
							}

							if (mHasUV)
							{
								bool lUnmappedUV;
								lMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
								lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
								lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
							}
						}
						++lVertexCount;
					}

				}//--------------------------------  for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)-------------------------------end 
				delete[] lVertices;
				delete[] lNormals;
				delete[] lUVs;
				delete[] lIndices;
			}//--------------------------if ( lMesh && !lMesh->GetUserDataPtr())-------------end 

		}//----------------if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)-------------end 

	}//----------------------if (lNodeAttribute)-------------end



	//FbxNode* lRootNode = lScene->GetRootNode();
	//int nodeNum = lRootNode->GetChildCount();

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FbxChildNode(pNode->GetChild(lChildIndex));
	}


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

	
	m_CustomCube.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_customPlane.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_scene3d.SetFbxScene3DViewMatrix(modelMatrix, viewMatrix);
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

	
	m_CustomCube.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_customPlane.SetModelViewMatrix(modelMatrix, viewMatrix);
	m_scene3d.SetFbxScene3DViewMatrix(modelMatrix, viewMatrix);

	if (m_loadingComplete)
	{
		if (!m_tracking)
		{
			// 少量旋转立方体。
			float curElapseTime= static_cast<float>(timer.GetElapsedSeconds()) * m_radiansPerSecond;
			m_angle += curElapseTime;

			Rotate(m_angle);
		
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
	DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineStateManager.GetPipelineStageByName(L"LightHLSLShader").Get()));
	m_commandList->SetPipelineState(m_pipelineStateManager.GetPipelineStageByName(L"LightHLSLShader").Get());

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

	


		DirectX::XMMATRIX curCubeMatrix;

		//使用第一张纹理，渲染第一个木地板
		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\Ground.jpg", 1, m_commandList);//banner.tif props1.tif//
		curCubeMatrix = XMMatrixScaling(5.0f, 0.05f, 5.0f)*XMMatrixTranslation(0, -1.9f, 0);
		m_customPlane.UpdatePlaneByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix);
		m_customPlane.DrawPlane(m_deviceResources->GetCurrentFrameIndex());

		//

		//使用第 个管线状态，渲染球体
		Microsoft::WRL::ComPtr<ID3D12PipelineState> lightShader = m_pipelineStateManager.GetPipelineStageByName(L"LightHLSLShader");
		m_commandList->SetPipelineState(lightShader.Get());

		m_constantPSBuffer.Update();
		m_constantPSBuffer.BindPSConstantBuffer(&m_cbvHeapManager, m_commandList, 2);


		curCubeMatrix =XMMatrixRotationY(-m_angle*0.5f)* XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0, -1.7f, -3);
		m_scene3d.UpdateAndDraw(&m_cbvHeapManager, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix, &m_textureManager, 1);
	
	
		Microsoft::WRL::ComPtr<ID3D12PipelineState> second = m_pipelineStateManager.GetPipelineStageByName(L"CubeMapHLSLShader");
		m_commandList->SetPipelineState(second.Get());

		m_textureManager.BindTexture(&m_cbvHeapManager, L"Assets\\CubeMap.dds", 1, m_commandList);
		curCubeMatrix = XMMatrixScaling(10.0f, 10.0f, 10.0f);
		m_CustomCube.UpdateCubeByIndex(&m_cbvHeapManager, 0, m_deviceResources->GetCurrentFrameIndex(), curCubeMatrix);
		m_CustomCube.DrawCube(m_deviceResources->GetCurrentFrameIndex());



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

void Sample3DSceneRenderer::OnHandleDBELoadBack(HRESULT res)
{
	if (FAILED(res))
	{
		
	}
	else
	{
		m_standby++;
		if (m_standby==2)
		{
			OnHandleCallBack();
		}
		
	}
	
}

void Sample3DSceneRenderer::OnHandleCallBack()
{
	// 创建立方体几何图形资源并上载到 GPU。
	auto createAssetslambda = [this]() {
		auto d3dDevice = m_deviceResources->GetD3DDevice();

		// 创建命令列表。
		//DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList = m_pipelineStateManager.GetGraphicsCommandList();
		NAME_D3D12_OBJECT(m_commandList);

		

		m_CustomCube.Initialize(d3dDevice, m_commandList, 1);
		m_CustomCube.CreateCubeVector();
		m_CustomCube.UpdateVertexAndIndexSubresources();

		m_customPlane.Initialize(d3dDevice, m_commandList, 1);
		m_customPlane.CreatePlaneVector();
		m_customPlane.UpdateVertexAndIndexSubresources();

		//FbxChildNode(lScene->GetRootNode());
		m_scene3d.CreatFbxScene3D(d3dDevice, m_commandList, lScene->GetRootNode());
		m_scene3d.UpdateFbxScene3DSubresources();

		// 为常量缓冲区创建描述符堆。
		{
			int custom_geometry_num = m_CustomCube.GetCubeNumber() + m_customPlane.GetPlaneNumber() + m_scene3d.GetAllEnity3DNumber();
			int numDescriptos = DX::c_frameCount*custom_geometry_num + 1 + 1;
			m_cbvHeapManager.Initialize(d3dDevice, numDescriptos);
		}

		
		// 创建顶点/索引缓冲区视图。
		

		m_CustomCube.UpLoadVextureConstantBuffer(&m_cbvHeapManager);
		m_CustomCube.CreateBufferView();

		m_customPlane.UpLoadVextureConstantBuffer(&m_cbvHeapManager);
		m_customPlane.CreateBufferView();

		m_scene3d.UpLoadFbxScene3DConstantBuffer(&m_cbvHeapManager);
		m_scene3d.CreateFbxScene3DBufferView();

#pragma region 加载纹理部分
		m_textureManager.Initialize(d3dDevice);
		m_textureManager.LoadTexture(&m_cbvHeapManager, L"Assets\\Ground.jpg", m_commandList);
		m_textureManager.LoadDDsTexture(&m_cbvHeapManager, L"Assets\\CubeMap.dds", m_commandList, D3D12_SRV_DIMENSION_TEXTURECUBE);
		//m_textureManager.LoadTexture(&m_cbvHeapManager, L"Assets\\banner.tif", m_commandList);//banner.tif props1.tif

		std::wstring fileStr = L"Assets\\";

		m_scene3d.GetAllImage();
		for (auto rit = m_scene3d.imageSet.crbegin(); rit != m_scene3d.imageSet.crend(); ++rit)
		{
			std::wstring imagePath = fileStr + *rit;

			m_textureManager.LoadTexture(&m_cbvHeapManager, imagePath.c_str(), m_commandList);//banner.tif props1.tif;

		}

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
		
		m_textureManager.RemoveUselessContent();
		m_CustomCube.RemoveUselessContent();
		m_customPlane.RemoveUselessContent();
		m_scene3d.RemoveFbxScene3DUselessContent();

		lSdkManager->Destroy();
	});
}