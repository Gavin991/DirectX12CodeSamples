#pragma once
#include <stack>
namespace DAE_Collada
{


	class NameArray
	{
	public: 
		NameArray()
		{
			m_Count = 0;
			m_ID = nullptr;
			m_Names = nullptr;
		}
		unsigned int        m_Count;
		std::string         m_ID;
		std::unique_ptr<std::string> m_Names;
	};

	class Param
	{
	public:
		Param()
		{
			m_Name = L"";
			m_Type = L"";
		}
		std::wstring        m_Name;
		std::wstring        m_Type;
	};

	class Accessor
	{
	public:
		Accessor()
		{
			m_Source = L"";
			m_Count = 0;
			m_Stride = 0;
			m_Params.clear();
		}
		std::wstring        m_Source;
		unsigned int        m_Count;
		unsigned int        m_Stride;
		//std::vector<Param>  m_Params;
		std::vector<std::unique_ptr<Param>>   m_Params;
	};

	class TechniqueCommon
	{
	public:
		TechniqueCommon()
		{
			m_Accessor = nullptr;			
		};
		std::unique_ptr<Accessor> m_Accessor;	
		
	};

	class  FloatArray
	{
	public:
		FloatArray()
		{
			m_Count = 0;
			m_ID = L"";
			m_Floats.clear();
		};
		unsigned int        m_Count;
		std::wstring 		m_ID;
		std::vector<float>  m_Floats;
	};

	class Source
	{
	public:
		Source() 
		{
			m_ID = L"";
			m_Float_Array = nullptr;
			//m_Name_Array = nullptr;
			m_TechniqueCommon = nullptr;			
		};
		std::wstring         m_ID;
		std::unique_ptr<FloatArray> m_Float_Array;
		//std::unique_ptr<NameArray> m_Name_Array;
		std::unique_ptr<TechniqueCommon> m_TechniqueCommon;
	};

	class Input
	{
	public:
		Input()
		{
			m_Semantic = L"";
			m_Source = L"";
			m_Offset = L"";
			m_Set == L"";
		}
		std::wstring         m_Semantic;
		std::wstring         m_Source;
		std::wstring         m_Offset;
		std::wstring         m_Set;
	};

	class Vertices
	{
	public:
		Vertices()
		{
			m_ID = L"";
			m_Inputs.clear();
		}
		std::wstring         m_ID;
		std::vector<std::unique_ptr<Input>>   m_Inputs;
	};

	class P
	{
	public:
		P()
		{
			m_Count = 0;
			m_Indices = nullptr;
		};
		unsigned int         m_Count;
		std::unique_ptr<unsigned short> m_Indices;
	};

	class Triangles
	{
	public:
		Triangles()
		{
			m_Count = 0;
			m_Material =  L"";
			m_Inputs.clear();
			m_P.clear();
		};
		unsigned int         m_Count;
		std::wstring		 m_Material;
		std::vector<std::unique_ptr<Input>>   m_Inputs;
		//std::unique_ptr<P>	 m_P;
		std::vector<unsigned short> m_P;
	};

	class Mesh
	{
	public: 
		Mesh()
		{
			m_Sources.clear();
			m_Vertices = nullptr;
			m_Triangles.clear();
		}
		std::vector<std::unique_ptr<Source>>			 m_Sources;
		std::unique_ptr<Vertices>	 m_Vertices;
		std::vector<std::unique_ptr<Triangles>>   m_Triangles;
	};

	class Geometry
	{		
	public:
		Geometry()
		{
			m_Mesh = nullptr;
		};
		std::unique_ptr<Mesh>	 m_Mesh;
	};

	class GeometryLibrary
	{
	public:
		GeometryLibrary() 
		{
			m_Geometries.clear();
		};
		std::vector<std::unique_ptr<Geometry>>    m_Geometries;
	};
	////----------------------------------------------//
	//class Mesh
	//{
	//	Vector<Source>      m_Sources;
	//	Vertices            m_Vertices;
	//	Vector<Triangles>   m_Triangles;
	//};
	////----------------------------------------------//

	class Collada 
	{
	public:
		Collada();	
		void LoadFromDBE(Platform::String^ filePath, std::function<void(HRESULT)> callBackFun);
		HRESULT ReadXml(Windows::Storage::Streams::IRandomAccessStream^ randomAccessReadStream);
		HRESULT ReadAttributes(IXmlReader* reader);

		GeometryLibrary    m_GeometryLibrary;

		std::unique_ptr<Geometry>	 tempGeometry;
		std::unique_ptr<Geometry> GetFlashGeometry()
		{
			std::unique_ptr<Geometry> tempObject(new Geometry());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Mesh>	tempMesh;
		std::unique_ptr<Mesh> GetFlashMesh()
		{
			std::unique_ptr<Mesh> tempObject(new Mesh());
			return tempObject;    // 返回unique_ptr
		}
		
		std::unique_ptr<Source>	tempSource;
		std::unique_ptr<Source> GetFlashSource()
		{
			std::unique_ptr<Source> tempObject(new Source());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<FloatArray>	tempFloatArray;
		std::unique_ptr<FloatArray> GetFlashFloatArray()
		{
			std::unique_ptr<FloatArray> tempObject(new FloatArray());
			return tempObject;    // 返回unique_ptr
		}
		
		std::unique_ptr<TechniqueCommon>	tempTechniqueCommon;
		std::unique_ptr<TechniqueCommon> GetFlashTechniqueCommon()
		{
			std::unique_ptr<TechniqueCommon> tempObject(new TechniqueCommon());
			return tempObject;    // 返回unique_ptr
		}	

		std::unique_ptr<Accessor>	tempAccessor;
		std::unique_ptr<Accessor> GetFlashAccessor()
		{
			std::unique_ptr<Accessor> tempObject(new Accessor());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Param>	tempParam;
		std::unique_ptr<Param> GetFlashParam()
		{
			std::unique_ptr<Param> tempObject(new Param());
			return tempObject;    // 返回unique_ptr
		}
		
		std::unique_ptr<Vertices>	tempVertices;
		std::unique_ptr<Vertices> GetFlashVertices()
		{
			std::unique_ptr<Vertices> tempObject(new Vertices());
			return tempObject;    // 返回unique_ptr
		}		

		std::unique_ptr<Triangles>	tempTriangles;
		std::unique_ptr<Triangles> GetFlashTriangles()
		{
			std::unique_ptr<Triangles> tempObject(new Triangles());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Input>	tempInput;
		std::unique_ptr<Input> GetFlashInput()
		{
			std::unique_ptr<Input> tempObject(new Input());
			return tempObject;    // 返回unique_ptr
		}

		
		void PushElement(PCWSTR element) { elementStack.push(element); };
		PCWSTR GetTopElement() { return elementStack.top();	}
		void PopElement() { elementStack.pop(); }



		const PCWSTR library_geometries_str =L"library_geometries";
		const PCWSTR geometry_str = L"geometry";
		const PCWSTR mesh_str = L"mesh";
		//const PCWSTR souce_str = L"souce";
		const PCWSTR technique_common_str = L"technique_common";
		const PCWSTR float_array_str = L"float_array";
		const PCWSTR accessor_str = L"accessor";
		const PCWSTR param_str = L"param";
		const PCWSTR triangles_str = L"triangles";
		const PCWSTR vertices_str = L"vertices";
		const PCWSTR input_str = L"input";
		const PCWSTR p_str = L"p";
		
		const PCWSTR id_str = L"id";
		const PCWSTR count_str = L"count";
		const PCWSTR material_str = L"material";
		const PCWSTR semantic_str = L"semantic";
		const PCWSTR offset_str = L"offset";
		const PCWSTR source_str = L"source";
		const PCWSTR set_str = L"set";
		const PCWSTR stride_str = L"stride";
		const PCWSTR name_str = L"name";
		const PCWSTR type_str = L"type";
	private:
		std::stack<PCWSTR> elementStack;
	};


}
