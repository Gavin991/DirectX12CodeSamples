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

		//DAE_Collada::Geometry temp;
		//temp.kao = 0;
		//GeometryLibrary tempgeo;
		std::unique_ptr<DAE_Collada::Geometry>	 temp;

		switch (nodeType)
		{
		case XmlNodeType_XmlDeclaration: //     XML 声明（例如，<?xml version='1.0'?>）。
										 //XmlTextbox->Text += L"XmlDeclaration\n";
										 //ChkHr(ReadAttributes(reader.Get()));
			break;

		case XmlNodeType_Element: //     元素（例如，<item>）。
			ChkHr(reader->GetPrefix(&prefix, &prefixSize));
			ChkHr(reader->GetLocalName(&localName, &localNameSize));

			//if (prefixSize > 0)
			//{
			//	XmlTextbox->Text += L"Element_prefix: ";
			//	ChkHr(ConcatToOutput(prefix, prefixSize));
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"\n";
			//}
			//else
			//{
			//	XmlTextbox->Text += L"Element: ";
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"\n";
			//}
			//ChkHr(ReadAttributes(reader.Get()));

			//if (reader->IsEmptyElement())
			//{
			//	XmlTextbox->Text += L" (empty)";
			//}

			PushElement(localName);
			res = wcscmp(localName, library_geometries_str);
			if (res == 0)
			{
				tempGeometry = GetFlashGeometry();
				//m_collada.PushElement(m_collada.library_geometries_str);
				//m_collada.m_GeometryLibrary.m_Geometries.push_back(std::move(m_collada.tempGeometry));				
			}

			res = wcscmp(localName, mesh_str);
			if (res == 0)
			{
				tempMesh = GetFlashMesh();
				//m_collada.PushElement(m_collada.mesh_str);						
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




			ChkHr(ReadAttributes(reader.Get()));

			break;

		case XmlNodeType_EndElement://     末尾元素标记（例如，</item>）。
			ChkHr(reader->GetPrefix(&prefix, &prefixSize));
			ChkHr(reader->GetLocalName(&localName, &localNameSize));
			//if (prefixSize > 0)
			//{
			//	XmlTextbox->Text += L"End Element_prefixSize: ";
			//	ChkHr(ConcatToOutput(prefix, prefixSize));
			//	XmlTextbox->Text += L":";
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"\n";
			//}
			//else
			//{
			//	XmlTextbox->Text += L"End Element: ";
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"\n";
			//}

			PopElement();
			res = wcscmp(localName, library_geometries_str);
			if (res == 0)
			{
				m_GeometryLibrary.m_Geometries.push_back(std::move(tempGeometry));
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

			//// geometry ----- mesh ----  vertices ----Input
			//res = wcscmp(localName, m_collada.input_str);
			//if (res == 0 && m_collada.tempVertices!=nullptr)
			//{
			//	res = -1;
			//	m_collada.tempVertices->m_Inputs.push_back(std::move(m_collada.tempInput));
			//}

			// geometry ----- mesh ---- triangles
			res = wcscmp(localName, triangles_str);
			if (res == 0 && tempMesh)
			{
				tempMesh->m_Triangles.push_back(std::move(tempTriangles));
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

			//if (prefixSize > 0)
			//{
			//	XmlTextbox->Text += L"Attr: ";
			//	ChkHr(ConcatToOutput(prefix, prefixSize));
			//	XmlTextbox->Text += L":";
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"=\"";
			//	ChkHr(ConcatToOutput(value, valueSize));
			//	XmlTextbox->Text += L"\"\n";
			//}
			//else
			//{
			//	XmlTextbox->Text += L"Attr: ";
			//	ChkHr(ConcatToOutput(localName, localNameSize));
			//	XmlTextbox->Text += L"=\"";
			//	ChkHr(ConcatToOutput(value, valueSize));
			//	XmlTextbox->Text += L"\"\n";
			//}


			PCWSTR strCurElement;
			int res = -1;

			strCurElement = GetTopElement();

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

			//res = wcscmp(strCurElement, m_collada.input_str);
			//if (res == 0)
			//{
			//	res = wcscmp(localName, m_collada.semantic_str);
			//	if (res == 0)
			//		m_collada.tempInput->m_Semantic = value;
			//	res = wcscmp(localName, m_collada.offset_str);
			//	if (res == 0)
			//		m_collada.tempInput->m_Offset = value;
			//	res = wcscmp(localName, m_collada.set_str);
			//	if (res == 0)
			//		m_collada.tempInput->m_Set = value;
			//	res = wcscmp(localName, m_collada.source_str);
			//	if (res == 0)
			//		m_collada.tempInput->m_Source = value;
			//}
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

			//int res = wcscmp(strCurElement, m_collada.float_array_str);
			//if (res == 0)
			//{
			//	res = wcscmp(m_collada.tempInput->m_Semantic, L"VERTEX");
			//	if (res == 0)
			//		m_collada.tempTriangles->m_Inputs.push_back(std::move(m_collada.tempInput));
			//}

			//int res = wcscmp(strCurElement, m_collada.input_str);
			//if (res == 0)
			//{
			//	res = wcscmp(m_collada.tempInput->m_Semantic, L"VERTEX");
			//	if (res == 0)
			//		m_collada.tempTriangles->m_Inputs.push_back(std::move(m_collada.tempInput));
			//}

			break;
		}

	}

	return hr;
}
