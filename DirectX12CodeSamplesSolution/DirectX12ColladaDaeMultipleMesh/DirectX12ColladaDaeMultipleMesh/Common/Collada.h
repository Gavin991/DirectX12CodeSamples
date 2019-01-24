#pragma once
#include <stack>
namespace DAE_Collada
{

	
	//class NameArray
	//{
	//public: 
	//	NameArray()
	//	{
	//		m_Count = 0;
	//		m_ID = nullptr;
	//		m_Names = nullptr;
	//	}
	//	unsigned int        m_Count;
	//	std::string         m_ID;
	//	std::unique_ptr<std::string> m_Names;
	//};

	//Collada的dae格式文件的Param节点存储，例如<param name="X" type="float"/>
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

	//accessor节点存储，例如<accessor count="52" stride="3" source="#Base-POSITION-array">
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
		std::vector<std::unique_ptr<Param>>   m_Params;
	};
	//technique_common节点存储，例如technique_common>
	class TechniqueCommon
	{
	public:
		TechniqueCommon()
		{
			m_Accessor = nullptr;			
		};
		std::unique_ptr<Accessor> m_Accessor;	
		
	};
	//float_array节点存储，例如<float_array id="Base-Normal0-array" count="558">
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

	//source节点存储，例如-<source id="Base-POSITION">
	class Source
	{
	public:
		Source() 
		{
			m_ID = L"";
			m_Float_Array = nullptr;		
			m_TechniqueCommon = nullptr;			
		};
		std::wstring         m_ID;
		std::unique_ptr<FloatArray> m_Float_Array;		
		std::unique_ptr<TechniqueCommon> m_TechniqueCommon;
	};

	//input节点内存存储，例如<input source="#Base-VERTEX" semantic="VERTEX" offset="0"/>
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

	//vertices节点内存存储，例如-<vertices id="Base-VERTEX">
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

	//p节点内存存储，例如<p>
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

	//triangles节点内存存储，例如-<triangles count="304" material="Tow_Gaussgun">
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
		std::vector<unsigned short> m_P;
	};

	//mesh节点内存存储，例如-<mesh>
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

	//geometry节点内存存储，例如-<geometry name="BaseMesh" id="Base-lib">
	class Geometry
	{		
	public:
		Geometry()
		{
			m_Mesh = nullptr;
			m_Name = L"";
			m_ID = L"";
		};
		std::unique_ptr<Mesh>	 m_Mesh;
		std::wstring			 m_Name;
		std::wstring			 m_ID;
	};

	//library_geometries节点内存存储，例如-<library_geometries>
	class GeometryLibrary
	{
	public:
		GeometryLibrary() 
		{
			m_Geometries.clear();
		};
		std::vector<std::unique_ptr<Geometry>>    m_Geometries;
	};

	//------------------这里是分割线--------------------------------------

	//instance_material节点内存存储，例如<instance_material target="#Tow_Gaussgun" symbol="Tow_Gaussgun"/>
	class InstanceMaterial
	{
	public:
		InstanceMaterial()
		{
			m_target = L"";
			m_symbol = L"";
		}
		std::wstring			 m_target;
		std::wstring			 m_symbol;
	};

	//instance_geometry节点内存存储，例如-<instance_geometry url="#Barrel-lib">
	class InstanceGeometry
	{
	public:
		InstanceGeometry()
		{
			m_url = L"";
			m_material = nullptr;
		}
		std::wstring			 m_url;
		std::unique_ptr<InstanceMaterial>	 m_material;
	};
	//matrix节点内存存储，例如-<matrix sid = "matrix">
	class Matrix
	{
	public:
		Matrix()
		{
			m_matrix.clear();
		}
		std::vector<float>   m_matrix;
	
	};
	//node节点内存存储，例如-<node name="Turret" id="Turret" sid="Turret">
	class Node
	{
	public:
		Node()
		{
			m_Name = L"";
			m_ID = L"";
			m_SID = L"";
			m_matrix = nullptr;
			m_InstanceGeometry = nullptr;
			m_nodes.clear();
		}
		std::wstring			 m_Name;
		std::wstring			 m_ID;
		std::wstring			 m_SID;		
		std::unique_ptr<Matrix>	 m_matrix;
		std::unique_ptr<InstanceGeometry>	 m_InstanceGeometry;
		std::vector<std::unique_ptr<Node>>    m_nodes;
	};


	//visual_scene节点内存存储，例如-<visual_scene name="Tow_Gauss3" id="Tow_Gauss3">
	class VisualScene
	{
	public: 
		VisualScene()
		{
			m_Name = L"";
			m_ID = L"";
			m_nodes.clear();
		}
		std::wstring			 m_Name;
		std::wstring			 m_ID;
		std::vector<std::unique_ptr<Node>>   m_nodes;
	};

	//library_geometries节点内存存储，例如-<library_geometries>
	class VisualSceneLibrary
	{
	public:
		VisualSceneLibrary()
		{
			m_VisualScene = nullptr;
		}
		std::unique_ptr<VisualScene>    m_VisualScene;
	};

	//------------------这里是分割线--------------------------------------
	//init_from节点内存存储，例如<init_from>file://D:/Project/Tower_defence/Textures/Tow_gaussguns.tif</init_from>
	class InitFrom
	{
	public:
		InitFrom()
		{
			m_path = L"";		
		}
		std::wstring			 m_path;
		
	};

	//image节点内存存储，例如-<image name="file6" id="file6-image">
	class Image
	{
	public:
		Image()
		{
			m_Name = L"";
			m_ID = L"";
			m_InitFrom = nullptr;
		}
		std::wstring			 m_Name;
		std::wstring			 m_ID;
		std::unique_ptr<InitFrom>    m_InitFrom;
	};

	//library_images节点内存存储，例如-<library_images>
	class ImageLibrary
	{
	public:
		ImageLibrary()
		{
			m_images.clear();
		}
		std::vector<std::unique_ptr<Image>>   m_images;
	};

	//------------------这里是分割线--------------------------------------

	//instance_effect节点内存存储，例如<instance_effect url="#Material #4-fx"/>
	class InstanceEffect
	{
	public:
		InstanceEffect()
		{
			m_url = L"";
		}
		std::wstring			 m_url;

	};

	//material节点内存存储，例如-<material name="Material #4" id="Material #4">
	class Material
	{
	public:
		Material()
		{
			m_Name = L"";
			m_ID = L"";
			m_InitFrom = nullptr;
		}
		std::wstring			 m_Name;
		std::wstring			 m_ID;
		std::unique_ptr<InstanceEffect>    m_InitFrom;
	};

	//library_materials节点内存存储，例如-<library_materials>
	class MaterialLibrary
	{
	public:
		MaterialLibrary()
		{
			m_materials.clear();
		}
		std::vector<std::unique_ptr<Material>>   m_materials;
	};

	//------------------这里是分割线--------------------------------------
	//texture节点内存存储，例如-<texture texcoord="CHANNEL0" texture="贴图 #1-image">
	class Texture
	{
	public:
		Texture()
		{
			m_texcoord = L"";
			m_texture = L"";		
		}
		std::wstring			 m_texcoord;
		std::wstring			 m_texture;
	};

	//color节点内存存储，例如<color sid = "emission">0.000000 0.000000 0.000000 1.000000< / color>
	class Color
	{
	public:
		Color()
		{
			m_SID = L"";
			m_rgba.clear();
		}
		std::wstring			 m_SID;
		std::vector<float>   m_rgba;
	};

	//effect节点内存存储，例如-<effect name="Material #4" id="Material #4-fx">
	class Effect
	{
	public:
		Effect()
		{
			m_Name = L"";
			m_ID = L"";
			m_emission = nullptr;
			m_ambient = nullptr;
			m_specular = nullptr;
			m_texture = nullptr;
			
		}
		std::wstring				m_Name;
		std::wstring				m_ID;
		std::unique_ptr<Color>		m_emission;
		std::unique_ptr<Color>		m_ambient;
		std::unique_ptr<Color>		m_specular;
		std::unique_ptr<Texture>    m_texture;
		float						m_shininess;
	};

	//library_effects节点内存存储，例如-<library_effects>
	class EffectLibrary
	{
	public:
		EffectLibrary()
		{
			m_effect.clear();
		}
		std::vector<std::unique_ptr<Effect>>   m_effect;
	};

	//------------------这里是分割线--------------------------------------

	//加载并解析Collada dae格式文件
	class Collada 
	{
	public:
		Collada();	
		void LoadFromDBE(Platform::String^ filePath, std::function<void(HRESULT)> callBackFun);
		HRESULT ReadXml(Windows::Storage::Streams::IRandomAccessStream^ randomAccessReadStream);
		HRESULT ReadAttributes(IXmlReader* reader);

		//存储的是网格
		GeometryLibrary			m_GeometryLibrary;
		//存储的是场景
		VisualSceneLibrary		m_VisualSceneLibrary;
		//存储的是贴图路径
		ImageLibrary			m_ImageLibrary;
		//存储的是材质路径
		MaterialLibrary			m_MaterialLibrary;
		//存储的是effect路径
		EffectLibrary			m_effectLibrary;

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

		//------------分隔线------------------------------------------------

		std::unique_ptr<VisualScene>	tempVisualScene;
		std::unique_ptr<VisualScene> GetFlashVisualScene()
		{
			std::unique_ptr<VisualScene> tempObject(new VisualScene());
			return tempObject;    // 返回unique_ptr
		}


		//std::unique_ptr<Node>	tempNode; Matrix InstanceGeometry
		std::unique_ptr<Node> GetFlashNode()
		{
			std::unique_ptr<Node> tempObject(new Node());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Matrix>	tempMatrix;
		std::unique_ptr<Matrix> GetFlashMatrix()
		{
			std::unique_ptr<Matrix> tempObject(new Matrix());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<InstanceGeometry>	tempInstanceGeometry;
		std::unique_ptr<InstanceGeometry> GetFlashInstanceGeometry()
		{
			std::unique_ptr<InstanceGeometry> tempObject(new InstanceGeometry());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<InstanceMaterial>	tempInstanceMaterial;
		std::unique_ptr<InstanceMaterial> GetFlashInstanceMaterial()
		{
			std::unique_ptr<InstanceMaterial> tempObject(new InstanceMaterial());
			return tempObject;    // 返回unique_ptr
		}

		//------------分隔线------------------------------------------------ 
		std::unique_ptr<Image>	tempImage;
		std::unique_ptr<Image> GetFlashImage()
		{
			std::unique_ptr<Image> tempObject(new Image());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<InitFrom>	tempInitFrom;
		std::unique_ptr<InitFrom> GetFlashInitFrom()
		{
			std::unique_ptr<InitFrom> tempObject(new InitFrom());
			return tempObject;    // 返回unique_ptr
		}
		//------------分隔线------------------------------------------------ 
		std::unique_ptr<Material>	tempMaterial;
		std::unique_ptr<Material> GetFlashMaterial()
		{
			std::unique_ptr<Material> tempObject(new Material());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<InstanceEffect>	tempInstanceEffect;
		std::unique_ptr<InstanceEffect> GetFlashInstanceEffect()
		{
			std::unique_ptr<InstanceEffect> tempObject(new InstanceEffect());
			return tempObject;    // 返回unique_ptr
		}
		//------------分隔线------------------------------------------------ 
		std::unique_ptr<Texture>	tempTexture;
		std::unique_ptr<Texture> GetFlashTexture()
		{
			std::unique_ptr<Texture> tempObject(new Texture());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Color>	tempColor;
		std::unique_ptr<Color> GetFlashColor()
		{
			std::unique_ptr<Color> tempObject(new Color());
			return tempObject;    // 返回unique_ptr
		}

		std::unique_ptr<Effect>	tempEffect;
		std::unique_ptr<Effect> GetFlashEffect()
		{
			std::unique_ptr<Effect> tempObject(new Effect());
			return tempObject;    // 返回unique_ptr
		}

		//------------分隔线------------------------------------------------ 
		void PushElement(PCWSTR element) { elementStack.push(element); };
		PCWSTR GetTopElement() { return elementStack.top();	}
		void PopElement() { elementStack.pop(); }

		void PushNode(std::unique_ptr<Node> node) {	nodeStack.push(std::move(node));};
		std::unique_ptr<Node> GetTopNode() { return 	std::move(nodeStack.top());  }
		void PopNode() { nodeStack.pop(); }

		const PCWSTR library_geometries_str =L"library_geometries";
		const PCWSTR geometry_str = L"geometry";
		const PCWSTR mesh_str = L"mesh";		
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

		const PCWSTR visual_scene_str = L"visual_scene";
		const PCWSTR node_str = L"node";
		const PCWSTR sid_str = L"sid";
		const PCWSTR matrix_str = L"matrix";
		const PCWSTR instance_geometry_str = L"instance_geometry";
		const PCWSTR url_str = L"url";
		const PCWSTR instance_material_str = L"instance_material";
		const PCWSTR symbol_str = L"symbol";
		const PCWSTR target_str = L"target";

		const PCWSTR image_str = L"image";
		const PCWSTR init_from_str = L"init_from";
		//const PCWSTR material_str = L"material";
		const PCWSTR instance_effect_str = L"instance_effect";

		const PCWSTR library_effect_str = L"library_effect";
		const PCWSTR texture_str = L"texture";
		const PCWSTR color_str = L"color";
		const PCWSTR effect_str = L"effect";
		const PCWSTR emission_str = L"emission";
		const PCWSTR ambient_str = L"ambient";
		const PCWSTR specular_str = L"specular";
		const PCWSTR texcoord_str = L"texcoord";
	private:
		std::stack<PCWSTR> elementStack;
		std::stack<std::unique_ptr<Node>> nodeStack;
	};


}
