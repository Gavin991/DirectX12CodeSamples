#include "pch.h"
#include "Collada.h"

using namespace DAE_Collada;

using namespace Platform;
//using namespace Windows::Foundation;
//using namespace Windows::Foundation::Collections;
//using namespace Windows::UI::Xaml;
//using namespace Windows::UI::Xaml::Controls;
//using namespace Windows::UI::Xaml::Controls::Primitives;
//using namespace Windows::UI::Xaml::Data;
//using namespace Windows::UI::Xaml::Input;
//using namespace Windows::UI::Xaml::Media;
//using namespace Windows::UI::Xaml::Navigation;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace concurrency;
using namespace Microsoft::WRL;

Collada::Collada()
{
	
}

void DAE_Collada::Collada::LoadFromDBE(Platform::String^ filePath, std::function<void(HRESULT)> callBackFun)
{
	StorageFolder^ installFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

	installFolder->Path;
	//String^ name = L"Assets\\Banner.xml";
	//create_task(installFolder->GetFileAsync(name)).then([=](StorageFile^ manifest) {

	//	manifest->DisplayName;
	//	//Do something with the manifest file  
	//});

	create_task(installFolder->GetFileAsync(filePath)).then([](StorageFile^ file)
	{

		return file->OpenAsync(FileAccessMode::Read);
	}).then([this, callBackFun](IRandomAccessStream^ readStream)
	{
		HRESULT hr = ReadXml(readStream);
		//if (FAILED(hr))
		//{
		//	//XmlTextbox->Text = "Exception occured while reading the xml file, and the error code is " + hr.ToString();
		//	//rootPage->NotifyUser("Exception occured!", NotifyType::ErrorMessage);
		//}
		if (callBackFun != nullptr)
		{
			callBackFun(hr);
		}
		OutputDebugString(L"----------------------------------------------\n");

	});
}

HRESULT DAE_Collada::Collada::ReadXml(Windows::Storage::Streams::IRandomAccessStream^ randomAccessReadStream)
{
	HRESULT hr = S_OK;
	ComPtr<IStream> readStream;
	ComPtr<IXmlReader> reader;
	XmlNodeType nodeType;

	ChkHr(::CreateStreamOverRandomAccessStream(randomAccessReadStream, IID_PPV_ARGS(&readStream)));
	ChkHr(::CreateXmlReader(IID_PPV_ARGS(&reader), nullptr));
	ChkHr(reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit));
	ChkHr(reader->SetInput(readStream.Get()));

	// Reads until there are no more nodes.
	while (S_OK == (hr = reader->Read(&nodeType)))
	{
		PCWSTR prefix = nullptr;
		PCWSTR localName = nullptr;
		PCWSTR value = nullptr;

		UINT prefixSize = 0;
		UINT localNameSize = 0;
		UINT valueSize = 0;

		int res = -1;

		switch (nodeType)
		{
		case XmlNodeType_XmlDeclaration: //     XML 声明（例如，<?xml version='1.0'?>）。
			//XmlTextbox->Text += L"XmlDeclaration\n";
			//ChkHr(ReadAttributes(reader.Get()));
			break;

		case XmlNodeType_Element: //     元素（例如，<item>）。
			ChkHr(reader->GetPrefix(&prefix, &prefixSize));
			ChkHr(reader->GetLocalName(&localName, &localNameSize));

			PushElement(localName);
			//res = wcscmp(localName, library_geometries_str);
			res = wcscmp(localName, geometry_str);
			if (res == 0)
			{
				tempGeometry = GetFlashGeometry();			
			}

			res = wcscmp(localName, mesh_str);
			if (res == 0)
			{
				tempMesh = GetFlashMesh();								
			}

			//geometry ----- mesh ---- source 
			res = wcscmp(localName, source_str);
			if (res == 0)
			{
				tempSource = GetFlashSource();
			}

			//geometry ----- mesh ---- source ----- float_array
			res = wcscmp(localName, float_array_str);
			if (res == 0)
			{
				tempFloatArray = GetFlashFloatArray();
			}

			//geometry ----- mesh ---- source ----- technique_common
			res = wcscmp(localName, technique_common_str);
			if (res == 0)
			{
				tempTechniqueCommon = GetFlashTechniqueCommon();
			}

			//geometry ----- mesh ---- source ----- technique_common ----- accessor
			res = wcscmp(localName, accessor_str);
			if (res == 0)
			{
				tempAccessor = GetFlashAccessor();
			}

			//geometry ----- mesh ---- source ----- technique_common ----- accessor ----- param
			res = wcscmp(localName, param_str);
			if (res == 0)
			{
				tempParam = GetFlashParam();
			}

			//geometry ----- mesh ----  vertices
			res = wcscmp(localName, vertices_str);
			if (res == 0)
			{
				tempVertices = GetFlashVertices();
			}

			// geometry ----- mesh ----  vertices ----Input
			res = wcscmp(localName, input_str);
			if (res == 0)
			{
				tempInput = GetFlashInput();

			}

			// geometry ----- mesh ---- triangles
			res = wcscmp(localName, triangles_str);
			if (res == 0)
			{
				tempTriangles = GetFlashTriangles();
			}

			//------分隔线-------------------------------------------------
			//visual_scene
			res = wcscmp(localName, visual_scene_str);
			if (res == 0)
			{
				tempVisualScene = GetFlashVisualScene();
			}

			//visual_scene --- node 
			res = wcscmp(localName, node_str);
			if (res == 0)
			{
				nodeStack.push(std::move(GetFlashNode()));				
			}

			//visual_scene --- node ---- matrix 
			res = wcscmp(localName, matrix_str);
			if (res == 0)
			{
				tempMatrix = GetFlashMatrix();
			}

			//visual_scene --- node ---- instance_geometry
			res = wcscmp(localName, instance_geometry_str);
			if (res == 0)
			{
				tempInstanceGeometry = GetFlashInstanceGeometry();
			}

			//visual_scene --- node ---- instance_geometry------instance_material
			res = wcscmp(localName, instance_material_str);
			if (res == 0)
			{
				tempInstanceMaterial = GetFlashInstanceMaterial();
			}

			//image
			res = wcscmp(localName, image_str);
			if (res == 0)
			{
				tempImage = GetFlashImage();
			}

			//image ----- imit_from
			res = wcscmp(localName, init_from_str);
			if (res == 0)
			{
				tempInitFrom = GetFlashInitFrom();
			}

			//material
			res = wcscmp(localName, material_str);
			if (res == 0)
			{
				tempMaterial = GetFlashMaterial();
			}

			//material
			res = wcscmp(localName, instance_effect_str);
			if (res == 0)
			{
				tempInstanceEffect = GetFlashInstanceEffect();
			}


			//effect
			res = wcscmp(localName, effect_str);
			if (res == 0)
			{
				tempEffect = GetFlashEffect();
			}

			//Color
			res = wcscmp(localName, color_str);
			if (res == 0)
			{
				tempColor = GetFlashColor();
			}

			//Texture
			res = wcscmp(localName, texture_str);
			if (res == 0)
			{
				tempTexture = GetFlashTexture();
			}

			ChkHr(ReadAttributes(reader.Get()));

			break;

		case XmlNodeType_EndElement://     末尾元素标记（例如，</item>）。
			ChkHr(reader->GetPrefix(&prefix, &prefixSize));
			ChkHr(reader->GetLocalName(&localName, &localNameSize));
			
			PopElement();
			//res = wcscmp(localName, library_geometries_str);
			res = wcscmp(localName, geometry_str);
			if (res == 0)
			{
				m_GeometryLibrary.m_Geometries.push_back(std::move(tempGeometry));
				int ccctv = 0;
			}

			res = wcscmp(localName, mesh_str);
			if (res == 0 && tempGeometry != nullptr)
			{
				tempGeometry->m_Mesh = std::move(tempMesh);
			}

			//geometry ----- mesh ---- source 
			res = wcscmp(localName, source_str);
			if (res == 0 && tempMesh != nullptr)
			{
				tempMesh->m_Sources.push_back(std::move(tempSource));
			}

			//geometry ----- mesh ---- source ----- float_array
			res = wcscmp(localName, float_array_str);
			if (res == 0 && tempSource != nullptr)
			{
				tempSource->m_Float_Array = std::move(tempFloatArray);
			}

			//geometry ----- mesh ---- source ----- technique_common
			res = wcscmp(localName, technique_common_str);
			if (res == 0 && tempSource != nullptr)
			{
				tempSource->m_TechniqueCommon = std::move(tempTechniqueCommon);
			}

			//geometry ----- mesh ---- source ----- technique_common ----- accessor
			res = wcscmp(localName, accessor_str);
			if (res == 0 && tempTechniqueCommon != nullptr)
			{
				tempTechniqueCommon->m_Accessor = std::move(tempAccessor);
			}

			//geometry ----- mesh  ----- vertices
			res = wcscmp(localName, vertices_str);
			if (res == 0 && tempMesh != nullptr)
			{
				tempMesh->m_Vertices = std::move(tempVertices);
			}			

			// geometry ----- mesh ---- triangles
			res = wcscmp(localName, triangles_str);
			if (res == 0 && tempMesh)
			{
				tempMesh->m_Triangles.push_back(std::move(tempTriangles));
			}

			//--------------分隔线---------------------------------------------

			res = wcscmp(localName, visual_scene_str);
			if (res == 0 && tempVisualScene)
			{
				//std::wstring k=tempVisualScene->m_ID;
				//std::wstring nameb = tempVisualScene->m_Name;
				m_VisualSceneLibrary.m_VisualScene = std::move(tempVisualScene);
			}

			res = wcscmp(localName, node_str);
			if (res == 0)
			{
				if (nodeStack.size()==0)
				{
					if (tempVisualScene)
					{
						//tempVisualScene->m_nodes.push_back(std::move(tempNode));
						
					}
				}
				else
				{
					std::unique_ptr<Node> sunNode = std::move(nodeStack.top());
					nodeStack.pop();
					if (nodeStack.size() == 0)
					{
						if (tempVisualScene)
						{
							tempVisualScene->m_nodes.push_back(std::move(sunNode));
							int yy = 0;
						}
					}
					else
					{
						std::unique_ptr<Node> fartherNode = std::move(nodeStack.top());
						nodeStack.pop();
						fartherNode->m_nodes.push_back(std::move(sunNode));
						nodeStack.push(std::move(fartherNode));
					}
					
				}
			}

			//visual_scene --- node ---- matrix 
			res = wcscmp(localName, matrix_str);
			if (res == 0 && nodeStack.size() > 0)
			{
				std::unique_ptr<Node> curNode = std::move(nodeStack.top());
				nodeStack.pop();
				curNode->m_matrix = std::move(tempMatrix);
				nodeStack.push(std::move(curNode));
			}


			//visual_scene --- node ---- instance_geometry
			res = wcscmp(localName, instance_geometry_str);
			if (res == 0 && nodeStack.size() > 0)
			{
				std::unique_ptr<Node> curNode = std::move(nodeStack.top());
				nodeStack.pop();
				curNode->m_InstanceGeometry = std::move(tempInstanceGeometry);
				nodeStack.push(std::move(curNode));
			}

			//image
			res = wcscmp(localName, image_str);
			if (res == 0)
			{
				m_ImageLibrary.m_images.push_back(std::move(tempImage));
			}


			//image ----- imit_from
			res = wcscmp(localName, init_from_str);
			if (res == 0 && tempImage)
			{
				tempImage->m_InitFrom = std::move(tempInitFrom);
			}

			//materia
			res = wcscmp(localName, material_str);
			if (res == 0)
			{
				m_MaterialLibrary.m_materials.push_back(std::move(tempMaterial));
			}


			//effect
			res = wcscmp(localName, effect_str);
			if (res == 0)
			{
				m_effectLibrary.m_effect.push_back(std::move(tempEffect));
			}

			//Color
			res = wcscmp(localName, color_str);
			if (res == 0&& tempEffect)
			{
				/*tempColor = GetFlashColor();*/
				//std::wstring rgb = tempColor->m_SID;
				res= wcscmp(tempColor->m_SID.c_str(), emission_str);
				if (res == 0&&tempColor)
				{
					tempEffect->m_emission = std::move(tempColor);
				}
				else
				{
					res = wcscmp(tempColor->m_SID.c_str(), ambient_str);
					if (res == 0&&tempColor)
					{
						tempEffect->m_ambient = std::move(tempColor);
					}
					else
					{
						res = wcscmp(tempColor->m_SID.c_str(), specular_str);
						if (res == 0&&tempColor)
						{
							tempEffect->m_specular = std::move(tempColor);
						}
					}
				
				}	
			}

			//Texture
			res = wcscmp(localName, texture_str);
			if (res == 0 && tempEffect)
			{
				tempEffect->m_texture = std::move(tempTexture);
			}

			break;




		case XmlNodeType_Text://     节点的文本内容。
			ChkHr(reader->GetValue(&value, &valueSize));
			//XmlTextbox->Text += L"Text: >";
			//ChkHr(ConcatToOutput(value, valueSize));
			//XmlTextbox->Text += L"<\n";

			PCWSTR strCurElement;
			//int res = -1;
			//geometry ----- mesh ---- source  ---- float_array =====> id
			strCurElement = GetTopElement();

			res = wcscmp(strCurElement, float_array_str);
			if (res == 0 && tempFloatArray != nullptr)
			{
				std::wstring s = value;
				//std::vector<std::wstring> tokens;
				std::wstring token;

				for_each(s.begin(), s.end(), [&](char c) {
					if (!isspace(c))
						token += c;
					else
					{
						if (token.length())
						{
							//tokens.push_back(token);
							float f = (float)_wtof(token.c_str());
							tempFloatArray->m_Floats.push_back(f);
						}
						token.clear();
					}
				});
				if (token.length())
				{
					//tokens.push_back(token);
					float f = (float)_wtof(token.c_str());
					tempFloatArray->m_Floats.push_back(f);
				}
				res = -1;
			}

			//geometry ----- mesh ----triangles ======> p
			res = wcscmp(strCurElement, p_str);
			if (res == 0 && tempTriangles)
			{
				std::wstring s = value;
				std::wstring token;

				for_each(s.begin(), s.end(), [&](char c) {
					if (!isspace(c))
						token += c;
					else
					{
						if (token.length())
						{
							unsigned short f = (unsigned short)_wtof(token.c_str());
							tempTriangles->m_P.push_back(f);
						}
						token.clear();
					}
				});
				if (token.length())
				{
					unsigned short f = (unsigned short)_wtof(token.c_str());
					tempTriangles->m_P.push_back(f);
				}
				res = -1;
			}


			//visual_scene --- node ---- matrix 
			res = wcscmp(strCurElement, matrix_str);
			if (res == 0 && tempMatrix)
			{
				std::wstring s = value;
				std::wstring token;

				for_each(s.begin(), s.end(), [&](char c) {
					if (!isspace(c))
						token += c;
					else
					{
						if (token.length())
						{
							float f =(float)_wtof(token.c_str());
							tempMatrix->m_matrix.push_back(f);
						}
						token.clear();
					}
				});
				if (token.length())
				{
					unsigned short f = (unsigned short)_wtof(token.c_str());
					tempMatrix->m_matrix.push_back(f);
				}
				res = -1;
			}


			res = wcscmp(strCurElement, init_from_str);
			if (res == 0 && tempInitFrom)
			{
				tempInitFrom->m_path = value;
			}

			//color
			res = wcscmp(strCurElement, color_str);
			if (res == 0 && tempColor)
			{
				std::wstring s = value;
				std::wstring token;

				for_each(s.begin(), s.end(), [&](char c) {
					if (!isspace(c))
						token += c;
					else
					{
						if (token.length())
						{
							float f = (float)_wtof(token.c_str());
							tempColor->m_rgba.push_back(f);
						}
						token.clear();
					}
				});
				if (token.length())
				{
					unsigned short f = (unsigned short)_wtof(token.c_str());
					tempColor->m_rgba.push_back(f);
				}
				res = -1;
			}

			break;

		case XmlNodeType_CDATA: //     CDATA 节（例如，<![CDATA[my escaped text]]>）
			ChkHr(reader->GetValue(&value, &valueSize));
			//XmlTextbox->Text += L"CDATA: ";
			//XmlTextbox->Text += ref new String(value, valueSize);
			//XmlTextbox->Text += L"\n";
			break;

		case XmlNodeType_ProcessingInstruction: //     处理指令（例如，<?pi test?>）
			ChkHr(reader->GetLocalName(&localName, &localNameSize));
			ChkHr(reader->GetValue(&value, &valueSize));
			//XmlTextbox->Text += L"Processing Instruction name:";
			//ChkHr(ConcatToOutput(localName, localNameSize));
			//XmlTextbox->Text += L"value:";
			//ChkHr(ConcatToOutput(value, valueSize));
			//XmlTextbox->Text += L"\n";
			break;

		case XmlNodeType_Comment: //     注释（例如，<!-- my comment -->）。
			ChkHr(reader->GetValue(&value, &valueSize));
			//XmlTextbox->Text += L"Comment: ";
			//ChkHr(ConcatToOutput(value, valueSize));
			//XmlTextbox->Text += L"\n";
			break;

		case XmlNodeType_DocumentType://     由以下标记指示的文档类型声明（例如，<!DOCTYPE...>）。		
			break;

		case XmlNodeType_Whitespace://     标记间的空白。
			ChkHr(reader->GetValue(&value, &valueSize));
			//XmlTextbox->Text += L"WhiteSpace: ";
			//ChkHr(ConcatToOutput(value, valueSize));
			//XmlTextbox->Text += L"\n";
			break;

		default:
			break;
		}
	}
	return hr;
}

HRESULT DAE_Collada::Collada::ReadAttributes(IXmlReader* reader)
{
	HRESULT hr = S_OK;

	ChkHr(reader->MoveToFirstAttribute());

	for (;;)
	{
		if (!reader->IsDefault())
		{
			PCWSTR prefix = nullptr;
			PCWSTR localName = nullptr;
			PCWSTR value = nullptr;

			UINT prefixSize = 0;
			UINT localNameSize = 0;
			UINT valueSize = 0;

			ChkHr(reader->GetPrefix(&prefix, &prefixSize));
			ChkHr(reader->GetLocalName(&localName, &localNameSize));
			ChkHr(reader->GetValue(&value, &valueSize));
			

			PCWSTR strCurElement;
			int res = -1;

			strCurElement = GetTopElement();

			//geometry
			res = wcscmp(strCurElement, geometry_str);
			if (res == 0)
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempGeometry)
				{
					tempGeometry->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && tempGeometry)
				{
					tempGeometry->m_ID = value;
				}
			}

			//geometry ----- mesh ---- source  ---- float_array =====> id,count;
			res = wcscmp(strCurElement, float_array_str);
			if (res == 0)
			{
				res = wcscmp(localName, id_str);
				if (res == 0 && tempFloatArray)
				{
					tempFloatArray->m_ID = value;
				}

				res = wcscmp(localName, count_str);
				if (res == 0 && tempFloatArray)
				{
					tempFloatArray->m_Count = _wtoi(value);
				}
			}

			//geometry ----- mesh ---- source  ---- technique_common ---- accessor  =====> count,stride,source;
			res = wcscmp(strCurElement, accessor_str);
			if (res == 0)
			{
				res = wcscmp(localName, count_str);
				if (res == 0 && tempAccessor)
				{
					tempAccessor->m_Count = _wtoi(value);
				}

				res = wcscmp(localName, stride_str);
				if (res == 0 && tempAccessor)
				{
					tempAccessor->m_Stride = _wtoi(value);
				}

				res = wcscmp(localName, source_str);
				if (res == 0 && tempAccessor)
				{
					tempAccessor->m_Source = value;
				}
			}

			//geometry ----- mesh ---- source ----- technique_common ----- accessor ----- param ====> name,type;
			res = wcscmp(strCurElement, param_str);
			if (res == 0)
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempParam)
				{
					tempParam->m_Name = value;
				}

				res = wcscmp(localName, type_str);
				if (res == 0 && tempParam)
				{
					tempParam->m_Type = value;
				}
			}

			//geometry ----- mesh ---- source  ====> name,type;
			res = wcscmp(strCurElement, source_str);
			if (res == 0)
			{
				res = wcscmp(localName, id_str);
				if (res == 0)
				{
					tempSource->m_ID = value;
				}
			}

			//geometry ----- mesh  ----- vertices ====> id
			res = wcscmp(strCurElement, vertices_str);
			if (res == 0)
			{
				res = wcscmp(localName, id_str);
				if (res == 0)
				{
					tempVertices->m_ID = value;
				}
			}

			// geometry ----- mesh ----  vertices ----Input  =====> source,semantic
			// geometry ----- mesh ---- triangles ----input  =====> source,semantic,offset ,set,
			res = wcscmp(strCurElement, input_str);
			if (res == 0)
			{
				// source
				res = wcscmp(localName, source_str);
				if (res == 0 && tempInput)
				{
					tempInput->m_Source = value;
				}
				// semantic
				res = wcscmp(localName, semantic_str);
				if (res == 0 && tempInput)
				{
					tempInput->m_Semantic = value;
				}
				//  offset,
				res = wcscmp(localName, offset_str);
				if (res == 0 && tempInput)
				{
					tempInput->m_Offset = value;
				}
				//  set,
				res = wcscmp(localName, set_str);
				if (res == 0 && tempInput)
				{
					tempInput->m_Set = value;
				}
			}

			// geometry ----- mesh ---- triangles ====> count, material
			res = wcscmp(strCurElement, triangles_str);
			if (res == 0)
			{
				res = wcscmp(localName, count_str);
				if (res == 0 && tempTriangles)
					tempTriangles->m_Count = _wtoi(value);
				res = wcscmp(localName, material_str);
				if (res == 0 && tempTriangles)
					tempTriangles->m_Material = value;
			}

			//visual_scene == == > name, id
			res = wcscmp(strCurElement, visual_scene_str);
			if (res == 0)
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempVisualScene)
				{
					tempVisualScene->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && tempVisualScene)
				{
					tempVisualScene->m_ID = value;
				}
			}

			//visual_scene --- node 
			res = wcscmp(strCurElement, node_str);
			if (res == 0)
			{
				std::unique_ptr<Node> curNode = std::move(nodeStack.top());
				nodeStack.pop();
				//fartherNode->m_nodes.push_back(std::move(sunNode));
				//nodeStack.push(std::move(fartherNode));

				res = wcscmp(localName, name_str);
				if (res == 0 && curNode)
				{
					curNode->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && curNode)
				{
					curNode->m_ID = value;
				}

				res = wcscmp(localName, sid_str);
				if (res == 0 && curNode)
				{
					curNode->m_SID = value;
				}

				nodeStack.push(std::move(curNode));
			}

			//visual_scene --- node ---- instance_geometry ====> Url
			res = wcscmp(strCurElement, instance_geometry_str);
			if (res == 0)
			{
				res = wcscmp(localName, url_str);
				if (res == 0 && tempInstanceGeometry)
				{
					tempInstanceGeometry->m_url = value;
				}				
			}

			//visual_scene --- node ---- instance_geometry ------instance_material ===>  target,symbol
			res = wcscmp(strCurElement, instance_material_str);
			if (res == 0)
			{
				res = wcscmp(localName, target_str);
				if (res == 0 && tempInstanceMaterial)
				{
					tempInstanceMaterial->m_target = value;
				}

				res = wcscmp(localName, symbol_str);
				if (res == 0 && tempInstanceMaterial)
				{
					tempInstanceMaterial->m_symbol = value;
				}
			}

			res = wcscmp(strCurElement, image_str);
			if (res == 0)
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempImage)
				{
					tempImage->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && tempImage)
				{
					tempImage->m_ID = value;
				}
			}


			//material-
			res = wcscmp(strCurElement, material_str);
			if (res == 0 )
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempMaterial)
				{
					tempMaterial->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && tempMaterial)
				{
					tempMaterial->m_ID = value;
				}
			}

			//material--- instance_effect
			res = wcscmp(strCurElement, instance_effect_str);
			if (res == 0 )
			{
				res = wcscmp(localName, url_str);
				if (res == 0 && tempInstanceEffect)
				{
					tempInstanceEffect->m_url = value;
				}
			}

			//effect
			res = wcscmp(strCurElement, effect_str);
			if (res == 0)
			{
				res = wcscmp(localName, name_str);
				if (res == 0 && tempEffect)
				{
					tempEffect->m_Name = value;
				}

				res = wcscmp(localName, id_str);
				if (res == 0 && tempEffect)
				{
					tempEffect->m_ID = value;
				}
			}

			//Color
			res = wcscmp(strCurElement, color_str);
			if (res == 0) 
			{
				res = wcscmp(localName, sid_str);
				if (res == 0 && tempColor)
				{
					tempColor->m_SID = value;
				}
			}
			

			//Texture
			res = wcscmp(strCurElement, texture_str);
			if (res == 0)
			{
				res = wcscmp(localName, texture_str);
				if (res == 0 && tempTexture)
				{
					tempTexture->m_texture = value;
				}

				res = wcscmp(localName, texcoord_str);
				if (res == 0 && tempTexture)
				{
					tempTexture->m_texcoord = value;
				}
			}

		}

		if (S_OK != (hr = reader->MoveToNextAttribute()))
		{
			PCWSTR strCurElement = GetTopElement();
			int res = -1;
			//geometry ----- mesh ---- source ----- technique_common ----- accessor ----- param ====> name,type;
			res = wcscmp(strCurElement, param_str);
			if (res == 0 && tempAccessor)
			{
				tempAccessor->m_Params.push_back(std::move(tempParam));
			}


			// geometry ----- mesh ----  vertices ----Input
			res = wcscmp(strCurElement, input_str);
			if (res == 0 && tempVertices != nullptr)
			{
				tempVertices->m_Inputs.push_back(std::move(tempInput));
			}
			// geometry -----triangles -----input
			if (res == 0 && tempTriangles != nullptr)
			{
				tempTriangles->m_Inputs.push_back(std::move(tempInput));
			}
			
			//visual_scene --- node ---- instance_geometry ------instance_material
			res = wcscmp(strCurElement, instance_material_str);
			if (res == 0 && tempInstanceGeometry != nullptr)
			{
				tempInstanceGeometry->m_material = std::move(tempInstanceMaterial);
			}


			//material--- instance_effect
			res = wcscmp(strCurElement, instance_effect_str);
			if (res == 0 && tempMaterial != nullptr )
			{
				tempMaterial->m_InitFrom=std::move(tempInstanceEffect);
			}

			break;
		}

	}

	return hr;
}
