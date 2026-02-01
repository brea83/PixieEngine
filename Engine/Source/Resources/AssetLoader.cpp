#include "BsPrecompileHeader.h"
#include "AssetLoader.h"
#include "StbImageWrapper.h"
#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef LoadImage

#include "Graphics/Primitives/Triangle.h"
#include "Graphics/Primitives/QuadMesh.h"
#include "Graphics/Primitives/Cube.h"
#include "Graphics/Primitives/CircleMesh.h"
#include "Scene/Components/MaterialInstance.h"

#include <chrono>

#include "Assimp/AssimpGlmHelpers.h"

#include "Resources/FileStream.h"
#include "Scene/GameObject.h"
#include "Resources/Prefab.h"

namespace Pixie
{
	//const static std::string s_SerializationVersion = "Version 0.1 of Bs Engine Serialization";
	std::unordered_map<std::string, std::shared_ptr<Resource>> AssetLoader::s_Resources;

	bool AssetLoader::IsMemoryAvailable(int minimumAvailableMb)
	{
		MEMORYSTATUSEX statusEx;
		statusEx.dwLength = sizeof(statusEx);
		GlobalMemoryStatusEx(&statusEx);
		
		//Logger::Core(LOG_INFO, "********************");
		//Logger::Core(LOG_INFO, "Physical Memory");
		//Logger::Core(LOG_INFO, "Total: {:d}", statusEx.ullTotalPhys / (1024 * 1024));
		//Logger::Core(LOG_INFO, "Available: {:d}", statusEx.ullAvailPhys / (1024 * 1024));
		

		bool result = minimumAvailableMb < statusEx.ullAvailPhys / (1024 * 1024);
		if (!result) Logger::Core(LOG_WARNING, "Available memory < minimum required. {:d} < {:d}", statusEx.ullAvailPhys / (1024 * 1024), minimumAvailableMb);
		//std::cout << "Available memory > minimum required == " << result << std::endl;
		return result;
	}

	std::shared_ptr<TextResource> AssetLoader::LoadTextFile(const std::string& filePath)
	{
		if (!IsMemoryAvailable(MimimumAvailableMb))
		{
			Logger::Core(LOG_ERROR, "LOAD TEXT FILE ERROR: available memory less than {:d}", MimimumAvailableMb);
			//std::cout << "" << std::endl;
			return nullptr;
		}

		if (s_Resources.find(filePath) != s_Resources.end())
		{
			auto textResourcePtr = std::dynamic_pointer_cast<TextResource>(s_Resources.at(filePath));
			if (textResourcePtr) return textResourcePtr;
		}

		// no stored text asset from that path so make one
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			Logger::Core(LOG_ERROR, "Failed to open file: {}", filePath);
			//std::cerr << "Failed to open file: " << filePath << std::endl;
			return nullptr;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();

		std::filesystem::path path = filePath;
		std::string fileName = path.filename().string();
		std::shared_ptr<TextResource> textSource = std::make_shared<TextResource>(TextResource(buffer.str(), fileName));
		s_Resources.emplace(fileName, textSource);

		return textSource;
	}

	bool AssetLoader::ReLoadTextFile(const std::string& filePath)
	{
		bool foundText = false;

		if (s_Resources.find(filePath) != s_Resources.end())
		{
			foundText = true;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			Logger::Core(LOG_ERROR, "Failed to open file: {}", filePath);
			//std::cerr << "Failed to open file: " << filePath << std::endl;
			return false;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();

		if (foundText)
		{
			auto textResourcePtr = std::dynamic_pointer_cast<TextResource>(s_Resources.at(filePath));
			if (textResourcePtr)
			{
				textResourcePtr->Text = buffer.str();
				return true;
			}
		}
		else
		{
			// we found no existing text file before so make and add a new one
			std::filesystem::path path = filePath;
			std::string fileName = path.filename().string();
			TextResource* textSource = new TextResource(buffer.str(), fileName);

			s_Resources.emplace(fileName, textSource);
			return true;
		}
		Logger::Core(LOG_ERROR, "ERROR, FILE FOUND IN ASSETS, BUT COULD NOT BE ACCESSED AS A TextResource.cpp");
		//std::cout << "ERROR, FILE FOUND IN ASSETS, BUT COULD NOT BE ACCESSED AS A TextResource.cpp" << std::endl;
		return false;
	}

	std::shared_ptr<Shader> AssetLoader::LoadShader(const std::string& vertPath, const std::string& fragPath)
	{
		if (!IsMemoryAvailable(MimimumAvailableMb))
		{
			Logger::Core(LOG_ERROR, "LOAD SHADER FILE ERROR: available memory less than {:d}", MimimumAvailableMb);
			//std::cout << "LOAD SHADER FILE ERROR: available memory less than " << std::endl;
			return nullptr;
		}

		if (s_Resources.find(vertPath + "|" + fragPath) != s_Resources.end())
		{
			auto shader = std::dynamic_pointer_cast<Shader>(s_Resources.at(vertPath + "|" + fragPath));
			if (shader) return shader;
			//return (Shader*)s_Resources.at(vertPath + "|" + fragPath);
		}

		std::shared_ptr<TextResource> vertexFile = LoadTextFile(vertPath);
		std::shared_ptr<TextResource> fragmentFile = LoadTextFile(fragPath);
		if (vertexFile == nullptr || fragmentFile == nullptr)
		{
			return nullptr;
		}
		std::string vertexGlsl = vertexFile->Text;
		std::string fragmentGlsl = fragmentFile->Text;


		// no stored shader made from those file paths, so make one
		std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertPath, fragPath);

		if (shader->IsValid())
		{
			s_Resources.emplace(vertPath + "|" + fragPath, shader);
		}
		return shader;
	}

	std::shared_ptr<Texture> AssetLoader::LoadTexture(const std::string& filePath, TextureType type)
	{
		if (!IsMemoryAvailable(MimimumAvailableMb))
		{
			Logger::Core(LOG_ERROR, "LOAD TEXTURE FILE ERROR: available memory less than {:d}", MimimumAvailableMb);
			//std::cout << "LOAD TEXTURE FILE ERROR: available memory less than " << std::endl;
			return nullptr;
		}

		//prep filepath
		std::string relativePath = "";
		ParsePathString(filePath, relativePath);

		std::filesystem::path relPath = filePath;
		relPath = relPath.relative_path();

		if (relativePath != "" && relativePath != filePath)
		{
			return LoadTextureParsedPath(relativePath, type);
		}

		return LoadTextureParsedPath(filePath, type);
	}


	std::filesystem::path AssetLoader::CreatePrefab(GameObject& baseObject)
	{
		std::shared_ptr<Prefab> newPrefab = std::make_shared<Prefab>(baseObject);
		
		std::filesystem::path path = newPrefab->GetPath();
		std::string pathString = path.string();

		if (s_Resources.find(pathString) != s_Resources.end())
		{
			Logger::Core(LOG_ERROR, "Cannot create new prefab named {}, this name is already taken. \
				Either change the object's name or use the OverWrite/Save-Changes for Prefab", path.stem().string());
			//std::cout << "Cannot create new prefab with this name it already exists." << std::endl;
			//std::cout << "Either change the object's name or use the OverWrite/Save-Changes for Prefab" << std::endl;
			return path;
		}

		
		FileStreamWriter fileOut(path);
		fileOut.WriteObject<Prefab>(*newPrefab);
		s_Resources.insert_or_assign(pathString, newPrefab);
		//s_Resources.emplace( pathString, newPrefab );

	}

	GameObject AssetLoader::LoadPrefab(const std::filesystem::path filePath, std::shared_ptr<Scene> scene)
	{
		if (filePath.extension() != "pmeta")
		{
			Logger::Core(LOG_WARNING, "tried to load prefab but it was not a .pmeta file, returning invalid GameObject.");
			//std::cout << "tried to load prefab but it was not a .pmeta file" << std::endl;
			return GameObject();
		}

		FileStreamReader fileIn(filePath);

		GameObject newCopy = scene->CreateEmptyGameObject("prefab Copy");

		Prefab instance(newCopy);
		fileIn.ReadObject(instance);

		
		return newCopy;
	}

	bool AssetLoader::LoadPrefab(const std::filesystem::path filePath, GameObject& rootObject)
	{
		if (filePath.extension() != "pmeta")
		{
			Logger::Core(LOG_WARNING, "tried to load prefab but it was not a .pmeta file.");
			//std::cout << "tried to load prefab but it was not a .pmeta file" << std::endl;
			return false;
		}

		FileStreamReader fileIn(filePath);

		Prefab instance(rootObject);
		fileIn.ReadObject(instance);

		return true;
	}


	bool AssetLoader::LoadMesh(GameObject& rootObject, MeshComponent& component, const std::filesystem::path filePath)
	{
		std::filesystem::path relativePath = ParsePathString(filePath);
		std::filesystem::path serializedPath = CheckForSerializedVersion(relativePath);
		if (serializedPath != "" && serializedPath.extension() == ".pmesh")
		{
			auto timerStart = std::chrono::high_resolution_clock::now();
			std::shared_ptr<Mesh> mesh = LoadSerializedMesh(serializedPath.string());
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);

			Logger::Core(LOG_INFO, "Loading serialized mesh took: {} milliseconds", duration.count());
			//std::cout << "Loading serialized mesh took: " << duration.count() << " milliseconds." << std::endl;

			if (mesh)
			{
				component.SetMesh(mesh);
				return true;
			}
		}

		if (filePath.extension() == ".fbx" || serializedPath.extension() == ".pmeta")
		{
			bool success = LoadFbx(filePath, rootObject);
			if (success) return true;
		}
		else if (filePath.extension() == ".obj")
		{
			std::shared_ptr<Mesh> mesh;
			auto timerStart = std::chrono::high_resolution_clock::now();
			mesh = LoadObj(filePath.string());
			auto timerEnd = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
			Logger::Core(LOG_INFO, "Loading Obj took: {} milliseconds", duration.count());
			//std::cout << "Loading Obj took: " << duration.count() << " milliseconds." << std::endl;
			
			if (mesh)
			{
				component.SetMesh(mesh);
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<Mesh> AssetLoader::LoadMesh(const std::filesystem::path filePath)
	{
		std::string serializedPath = CheckForSerializedVersion(filePath.string());
		if (serializedPath != "")
		{
			auto timerStart = std::chrono::high_resolution_clock::now();
			std::shared_ptr<Mesh> mesh = LoadSerializedMesh(serializedPath);
			auto timerEnd = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
			Logger::Core(LOG_INFO, "Loading serialized mesh took: {} milliseconds", duration.count());
			//std::cout << "Loading serialized mesh took: " << duration.count() << " milliseconds." << std::endl;

			return mesh;
		}

		std::shared_ptr<Mesh> mesh;
		if (filePath.extension() == ".fbx")
		{

			//return LoadFbx(filePath)
		}
		else if (filePath.extension() == ".obj")
		{
			auto timerStart = std::chrono::high_resolution_clock::now();
			mesh = LoadObj(filePath.string());
			auto timerEnd = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
			Logger::Core(LOG_INFO, "Loading Obj took: {} milliseconds", duration.count());
			//std::cout << "Loading Obj took: " << duration.count() << " milliseconds." << std::endl;
		}

		return mesh;
	}

	std::shared_ptr<Mesh> AssetLoader::LoadPrimitive(PrimitiveMeshType primitiveType)
	{
		switch (primitiveType)
		{
		case PrimitiveMeshType::Triangle:
		{
			if (s_Resources.find("PrimitiveMesh_Triangle") != s_Resources.end())
			{
				auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at("PrimitiveMesh_Triangle"));
				if (resourcePtr) return resourcePtr;
			}
			std::shared_ptr<Mesh> mesh = std::make_shared<Triangle>();

			s_Resources.emplace("PrimitiveMesh_Triangle", mesh);
			return mesh;
		}
		case PrimitiveMeshType::Quad:
		{
			if (s_Resources.find("PrimitiveMesh_Quad") != s_Resources.end())
			{
				auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at("PrimitiveMesh_Quad"));
				if (resourcePtr) return resourcePtr;
			}
			std::shared_ptr<Mesh> mesh = std::make_shared<Quad>();

			s_Resources.emplace("PrimitiveMesh_Quad", mesh);
			return mesh;
		}
		case PrimitiveMeshType::Cube:
		{
			if (s_Resources.find("PrimitiveMesh_Cube") != s_Resources.end())
			{
				auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at("PrimitiveMesh_Cube"));
				if (resourcePtr) return resourcePtr;
			}
			std::shared_ptr<Mesh> mesh = LoadMesh("../Assets/Meshes/Cube.obj");//std::make_shared<Cube>();
				
			s_Resources.emplace("PrimitiveMesh_Cube", mesh);
			return mesh;
		}
		case PrimitiveMeshType::Circle:
		{
			if (s_Resources.find("PrimitiveMesh_Circle") != s_Resources.end())
			{
				auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at("PrimitiveMesh_Circle"));
				if (resourcePtr) return resourcePtr;
			}
			std::shared_ptr<Mesh> mesh = std::make_shared<CircleMesh>();

			s_Resources.emplace("PrimitiveMesh_Circle", mesh);
			return mesh;
		}
		default:
			break;
		}
		return std::shared_ptr<Mesh>();
	}

	std::shared_ptr<Mesh> AssetLoader::LoadObj(const std::string& filePath, const std::string& textureFilePath)
	{
		if (!IsMemoryAvailable(MimimumAvailableMb))
		{
			Logger::Core(LOG_ERROR, "LOAD OBJ FILE ERROR: available memory less than {:d}", MimimumAvailableMb);
			//std::cout << "LOAD OBJ FILE ERROR: available memory less than " << std::endl;
			return nullptr;
		}

		if (s_Resources.find(filePath) != s_Resources.end())
		{
			auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at(filePath));
			if (resourcePtr) return resourcePtr;
		}

		std::ifstream file(filePath);

		if (!file.is_open())
		{
			std::cerr << "Failed to open file: " << filePath << std::endl;
			return nullptr;
		}
		std::string name = filePath.substr(filePath.find_last_of('/') + 1, filePath.find_last_of("."));

		Logger::Core(LOG_TRACE, ":::::::::::::::::::::::::::::::::::::::::::::");
		Logger::Core(LOG_TRACE, "IMPORTING OBJ: {}", name);
		Logger::Core(LOG_TRACE, "FilePath: {}", filePath);
		//std::cout << ":::::::::::::::::::::::::::::::::::::::::::::#" << std::endl;
		//std::cout << "IMPORTING OBJ " << name << std::endl;
		//std::cout << " FilePath: " << filePath << std::endl;
		std::string line;

		std::vector<Mesh::Vertex> vertices;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<unsigned int> indices;

		while (std::getline(file, line))
		{
			std::istringstream lineStream(line);
			std::string firstWord;
			lineStream >> firstWord;

			if (firstWord == "v")
			{
				// do vertex stuff
				glm::vec3 position;
				lineStream >> position.x >> position.y >> position.z;
				positions.push_back(position);
			}

			if (firstWord == "vt")
			{
				// texture coordinates
				glm::vec2 uv;
				lineStream >> uv.x >> uv.y;
				uvs.push_back(uv);
			}

			if (firstWord == "vn")
			{
				// normals
				glm::vec3 normal;
				lineStream >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}

			//if (firstWord == "vp")
			//{
			//	// parameter?
			//}

			if (firstWord == "f")
			{
				// faces data
				// need to double check but I think all formal exports require faces to come After all the other data
				//this will probably not get us quite enough spaces reserved, but it is probably closer than the empty init
				if (vertices.capacity() < positions.size()) vertices.reserve(positions.size());

				// format vertex_index/texture_index/normal_index
				// -1 referring to the last element of vertex list.

				std::vector<Mesh::ObjPackedIndices> objIndices;
				std::string word;
				while (lineStream >> word)
				{
					std::istringstream wordStream(word);
					std::string positionString, uvString, normalString;

					int relativePosIndex;
					int relativeUvIndex;
					int relativeNormalIndex;

					if (std::getline(wordStream, positionString, '/'))
					{
						relativePosIndex = std::atoi(positionString.c_str());
						relativePosIndex = std::max(0, relativePosIndex - 1);
					}
					else
					{
						relativePosIndex = 0;
					}

					if (std::getline(wordStream, uvString, '/'))
					{
						relativeUvIndex = std::atoi(uvString.c_str());
						relativeUvIndex = std::max(0, relativeUvIndex - 1);
					}
					else
					{
						relativeUvIndex = relativePosIndex;
					}

					if (std::getline(wordStream, normalString, '/'))
					{
						relativeNormalIndex = std::atoi(normalString.c_str());
						relativeNormalIndex = std::max(0, relativeNormalIndex - 1);
					}
					else
					{
						relativeNormalIndex = relativePosIndex;
					}

					int positionIndex, normalIndex, uvIndex;
					positionIndex = (relativePosIndex >= 0) ? relativePosIndex : positions.size() + relativePosIndex;
					uvIndex = (relativeUvIndex >= 0) ? relativeUvIndex : uvs.size() + relativeUvIndex;
					normalIndex = (relativeNormalIndex >= 0) ? relativeNormalIndex : normals.size() + relativeNormalIndex;

					objIndices.push_back(Mesh::ObjPackedIndices{ positionIndex, uvIndex, normalIndex });
				}

				//triangulate assuming n >3-gons are convex and coplanar
				for (size_t i = 1; i + 1 < objIndices.size(); i++)
				{
					const Mesh::ObjPackedIndices* point[3] = { &objIndices[0], &objIndices[i], &objIndices[i + 1] };

					//https://wikis.khronos.org/opengl/Calculating_a_Surface_Normal
					// U and V are the vectors used to calculate surface normal
					// U is point2 - point1 V is point3 - point1
					// normal is U cross V

					glm::vec3 U(positions[point[1]->Position] - positions[point[0]->Position]);
					glm::vec3 V(positions[point[2]->Position] - positions[point[0]->Position]);
					glm::vec3 faceNormal = glm::normalize(glm::cross(U, V));

					// make the vertex for the mesh

					for (size_t j = 0; j < 3; j++)
					{
						Mesh::Vertex vertex;

						vertex.Position = positions[point[j]->Position];
						vertex.Normal = (point[j]->Normal != 0 && normals.size() > 0) ? normals[point[j]->Normal] : faceNormal;
						if (uvs.size() > 0) vertex.UV1 = uvs[point[j]->Uv];

						// check if identical vertex exists to use its index instead
						auto found = std::find(vertices.begin(), vertices.end(), vertex);

						if (found != vertices.end())
						{
							size_t index = distance(vertices.begin(), found);
							indices.push_back(index);
						}
						else
						{
							indices.push_back(vertices.size());
							vertices.push_back(vertex);
						}
					}
				}
			}
		}
		file.close();

		for (int i = 0; i < indices.size(); i += 3)
		{
			//the three verts of our tri
			unsigned int index0 = indices[i];
			unsigned int index1 = indices[i + 1];
			unsigned int index2 = indices[i + 2];

			Mesh::Vertex& v0 = vertices[index0];
			Mesh::Vertex& v1 = vertices[index1];
			Mesh::Vertex& v2 = vertices[index2];

			//edges of the tri
			glm::vec3 edge1 = v1.Position - v0.Position;
			glm::vec3 edge2 = v2.Position - v0.Position;

			// uv deltas
			glm::vec2 deltaUV1 = v1.UV1 - v0.UV1;
			glm::vec2 deltaUV2 = v2.UV1 - v0.UV1;


			float denominator = (deltaUV1.x * deltaUV2.y) - (deltaUV1.y * deltaUV2.x);
			denominator = 1.0f / denominator;

			glm::vec3 triTangent = ((edge1 * deltaUV2.y) - (edge2 * deltaUV1.y)) * denominator;

			v0.Tangent = triTangent;
			v1.Tangent = triTangent;
			v2.Tangent = triTangent;

			v0.OrthagonalizeTangent();
			v1.OrthagonalizeTangent();
			v2.OrthagonalizeTangent();

			v0.CalculateBitangent();
			v1.CalculateBitangent();
			v2.CalculateBitangent();
		}

		std::shared_ptr<Mesh> mesh;

		mesh = std::make_shared<Mesh>(vertices, indices, name);

		if (mesh == nullptr) return nullptr;

		std::string serializedPath = SerializeMesh(filePath, mesh);
		if (serializedPath != "")
		{
			s_Resources.emplace(serializedPath, mesh);
			return mesh;
		}

		s_Resources.emplace(filePath, mesh);
		return mesh;
	}

	bool AssetLoader::LoadFbx(const std::filesystem::path filePath, GameObject& rootObject)
	{
		//check for matching fbx prefab
		std::filesystem::path searchPath("../Assets/Meshes/filename.pmeta");
		searchPath.replace_filename(filePath.stem());

		if (s_Resources.find(searchPath.string()) != s_Resources.end())
		{
			auto resourcePtr = std::dynamic_pointer_cast<Prefab>(s_Resources.at(filePath.string()));
			
			if (resourcePtr)
			{
				LoadPrefab(searchPath, rootObject);
				return true;
			}
		}
		//LoadModelAssimp(filePath);

		Assimp::Importer importer;
		unsigned int processFlags =
			aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
			aiProcess_GenSmoothNormals |
			aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
			//aiProcess_ValidateDataStructure  | // perform a full validation of the loader's output
			aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
			//aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space (by default right-handed, for OpenGL)
			//aiProcess_SortByPType | // ?
			aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
			//aiProcess_RemoveRedundantMaterials | // remove redundant materials
			//aiProcess_FindDegenerates | // remove degenerated polygons from the import
			//aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
			//aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
			//aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
			//aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
			aiProcess_OptimizeMeshes | // join small meshes, if possible;
			//aiProcess_OptimizeGraph |
			aiProcess_PreTransformVertices | //-- fixes the transformation issue.
			//aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
			0;
		const aiScene* assimpScene = importer.ReadFile(filePath.string(), processFlags);

		if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
		{
			Logger::Core(LOG_ERROR, "ERROR::ASSIMP:: {}", importer.GetErrorString());
			//std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return false;
		}

		double factor(0.0);
		if (assimpScene->mMetaData)
		{
			assimpScene->mMetaData->Get("UnitScaleFactor", factor);
			Logger::Core(LOG_TRACE, "Scene Unit Scale Factor is {}", factor);
			//std::cout << "Scene Unit Scale Factor is " << factor << std::endl;
		}

		Logger::Core(LOG_TRACE, ":::::::::::::::::::::::::::::::::::::::::::::");
		Logger::Core(LOG_TRACE, "IMPORTING: {}", filePath.string());
		//std::cout << ":::::::::::::::::::::::::::::::::::::::::::::#" << std::endl;
		//std::cout << "IMPORTING " << filePath << std::endl;

		//std::vector<Mesh> meshes;
		//meshes.reserve(assimpScene->mNumMeshes);
		NameComponent& name = rootObject.GetOrAddComponent<NameComponent>();
		name.Name = filePath.stem().string();

		std::shared_ptr<Scene> gameScene = rootObject.GetScene();
		aiNode* rootNode = assimpScene->mRootNode;

		factor = 0.0;
		if (rootNode->mMetaData)
		{
			rootNode->mMetaData->Get("UnitScaleFactor", factor);
			Logger::Core(LOG_TRACE, "Node Unit Scale Factor is {}", factor);
			//std::cout << "Node Unit Scale Factor is " << factor << std::endl;
		}

		ProcessNode(rootObject, rootNode, assimpScene, gameScene, name.Name);
		return true;
	}

	void AssetLoader::CleanUp()
	{
		std::vector<std::string> resourcesToDelete;
		for (auto pair : s_Resources)
		{
			int useCount = pair.second.use_count();
			//Logger::Core(LOG_TRACE, "{} has {} live pointers", pair.first, std::to_string(useCount));
			if (useCount <= 2)
			{
				// two uses means one is this for loop and the other is the instance in s_Resources ie: nothing outside the loader is using it
				resourcesToDelete.emplace_back(pair.first);
			}
		}

		for (std::string key : resourcesToDelete)
		{
			s_Resources.erase(key);
		}
	}

	void AssetLoader::ParsePathString(const std::string& inPath, std::string& outPath)
	{
		//prep filepath
		std::filesystem::path path = inPath;
		bool bIsInAssetsFolderPath{ false };
		outPath = "../";
		
		if (inPath.substr(0,9) != "../Assets")
		{
			for (auto part : path)
			{
				if (part == "Assets")
				{
					bIsInAssetsFolderPath = true;
				}

				if (bIsInAssetsFolderPath)
				{
					outPath += part.string();

					if (part.has_extension())
						continue;

					outPath += "/";
				}
				//Logger::Core(LOG_DEBUG, "{}", part.string());
			}

			//outPath = "../Assets" + inPath.substr(inPath.find("Assets"));
			return;
		}

	
		outPath = inPath;
	}

	std::filesystem::path AssetLoader::ParsePathString(const std::filesystem::path& inPath)
	{
		bool bIsInAssetsFolderPath{ false };
		std::filesystem::path outPath = "../";

		for (auto part : inPath)
		{
			if (part == "Assets")
			{
				bIsInAssetsFolderPath = true;
			}

			if (bIsInAssetsFolderPath)
			{
				outPath += part.string();

				if (part.has_extension())
					continue;

				outPath += "/";
			}
			//Logger::Core(LOG_DEBUG, "{}", part.string());
		}

		return bIsInAssetsFolderPath ? outPath : inPath;
		
	}

	std::shared_ptr<Texture> AssetLoader::LoadTextureParsedPath(const std::string& filePath, TextureType type)
	{
		if (s_Resources.find(filePath) != s_Resources.end())
		{
			auto texture = std::dynamic_pointer_cast<Texture>(s_Resources.at(filePath));
			if (texture) return texture;
		}

		StbImageData data;
		StbImageWrapper::LoadImage(filePath, data);

		if (!data.BLoadSuccess) return nullptr;

		std::shared_ptr<Texture> texture = std::make_shared<Texture>(data, type);

		if (texture->TextureObject == 0) return nullptr;

		s_Resources.emplace(filePath, texture);
		return texture;
	}

	std::string AssetLoader::CheckForSerializedVersion(const std::filesystem::path& filePath)
	{

		std::filesystem::path serializedPath = filePath;
		serializedPath.replace_extension("pmesh");

		if (s_Resources.find(serializedPath.string()) != s_Resources.end()) 
			return serializedPath.string();

		std::ifstream mesh(serializedPath);
		if (mesh.is_open())
		{
			mesh.close();
			return serializedPath.string();
		}

		// no pmesh file found check for prefab for fbx type mesh
		serializedPath.replace_extension("pmeta");
		std::ifstream file(serializedPath);

		if (file.is_open())
		{
			file.close();
			return serializedPath.string();;
		}

		std::cerr << "Failed to open file as either pmesh or pmeta: " << filePath << std::endl;
		return "";
	}

	std::string AssetLoader::SerializeMesh(const std::string& filePath, std::shared_ptr<Mesh> mesh)
	{
		std::string serializedPath = filePath.substr(0, filePath.find_last_of('.'));
		serializedPath.append(".pmesh");

		FileStreamWriter outFile(serializedPath);
		outFile.WriteObject<Mesh>(*mesh);

		return serializedPath;
	}

	std::shared_ptr<Mesh> AssetLoader::LoadSerializedMesh(const std::string& filePath)
	{
		if (s_Resources.find(filePath) != s_Resources.end())
		{
			auto resourcePtr = std::dynamic_pointer_cast<Mesh>(s_Resources.at(filePath));
			if (resourcePtr) return resourcePtr;
		}

		Mesh mesh;

		FileStreamReader fileIn(filePath);
		fileIn.ReadObject<Mesh>(mesh);
		
		return std::make_shared<Mesh>(mesh.GetVertices(), mesh.GetIndices());
		
	}

	//aiMatrix4x4 AssetLoader::CombineTransformsToRoot(aiNode* parentNode, aiNode* childNode)
	//{
	//	if (parentNode)
	//	{
	//		return (CombineTransformsToRoot(parentNode->mParent, parentNode)) * childNode->mTransformation.Inverse();
	//		// example expansion of a root node with three levels of children
	//		// ((rootTransform) * child1) * child2) * child3
	//	}

	//	return childNode->mTransformation.Inverse();

	//}

	std::shared_ptr<Mesh> AssetLoader::ProcessMesh(aiMesh* mesh, const aiScene* assimpScene)
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{

			Mesh::Vertex vertex;

			//process position 
			aiVector3D sourcePosition = mesh->mVertices[i];
			vertex.Position = glm::vec3(sourcePosition.x, sourcePosition.y, sourcePosition.z);

			// normals 
			aiVector3D sourceNormal = mesh->mNormals[i];
			vertex.Normal = glm::vec3(sourceNormal.x, sourceNormal.y, sourceNormal.z);

			// texture uvs
			if (mesh->mTextureCoords[0])
			{
				aiVector3D sourceUV = mesh->mTextureCoords[0][i];
				vertex.UV1 = glm::vec2(sourceUV.x, sourceUV.y);
			}
			else
			{
				vertex.UV1 = glm::vec2(0.0f, 0.0f);
			}

			// colors
			if (mesh->mColors[0])
			{
				aiColor4D* sourceColor = mesh->mColors[i];
				vertex.Color = glm::vec3(sourceColor->r, sourceColor->g, sourceColor->b);
			}
			else
			{
				vertex.Color = vertex.Normal;
			}

			// tangents
			vertex.Tangent.x = mesh->mTangents[i].x;
			vertex.Tangent.y = mesh->mTangents[i].y;
			vertex.Tangent.z = mesh->mTangents[i].z;

			vertex.BiTangent = glm::cross(vertex.Normal, vertex.Tangent);

			vertices.emplace_back(vertex);
		}
		// process indices

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.emplace_back(face.mIndices[j]);
			}
		}
		
		return std::make_shared<Mesh>(vertices, indices, mesh->mName.C_Str());
	}

	bool AssetLoader::ProcessTextures(MaterialInstance& objectMaterial, aiMesh* mesh, const aiScene* assimpScene)
	{
		std::vector<std::shared_ptr<Texture>> textures;

		// process materials

		aiMaterial* material = assimpScene->mMaterials[mesh->mMaterialIndex];

		if (material == nullptr) return false;
		

		//std::vector<std::shared_ptr<Texture>> diffuseMaps = LoadMaterialTextures(material,
		//	aiTextureType_DIFFUSE, TextureType::Diffuse);
		std::string diffusePath;
		std::shared_ptr<Texture> diffuseMap = GetTextureFromMaterial(material, aiTextureType_DIFFUSE, TextureType::Diffuse, diffusePath);

		std::string normalPath;
		std::shared_ptr<Texture> normalMap = GetTextureFromMaterial(material, aiTextureType_NORMALS, TextureType::Normal, normalPath);

		std::string metallicPath;
		std::shared_ptr<Texture> metallicMap = GetTextureFromMaterial(material, aiTextureType_METALNESS, TextureType::Metalness, metallicPath);
		std::string specularPath;
		std::shared_ptr<Texture> specularMAp = GetTextureFromMaterial(material, aiTextureType_SPECULAR, TextureType::Specular, specularPath);
		
		if (diffuseMap)
		{
			objectMaterial.BaseMap = diffuseMap;
			objectMaterial.BaseMapPath = diffusePath;
		}

		if (normalMap)
		{
			objectMaterial.NormalMap = normalMap;
			objectMaterial.NormalMapPath = normalPath;
		}

		if (metallicMap)
		{
			objectMaterial.MetallicMap = metallicMap;
			objectMaterial.MetallicMapPath = metallicPath;
		}

		if (specularMAp)
		{
			objectMaterial.SpecularMap = specularMAp;
			objectMaterial.SpecularMapPath = specularPath;
		}

		return true;
	}

	std::shared_ptr<Texture> AssetLoader::GetTextureFromMaterial(aiMaterial* material, aiTextureType type, TextureType pixieTextureType, std::string& outFilePath)
	{

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString filePath;
			material->GetTexture(type, i, &filePath);

			/*std::string typeName = TextureTypeToString.at(bsTextureType);
			Logger::Core(LOG_TRACE, "TEXTURE PATH: {}", filePath.C_Str() );
			Logger::Core(LOG_TRACE, "TYPE: {}", typeName);
			*/
			std::shared_ptr<Texture> texture = AssetLoader::LoadTexture(filePath.C_Str());
			if (texture == nullptr) continue;

			texture->Type = pixieTextureType;
			outFilePath = filePath.C_Str();
			return texture;
		}
		return nullptr;
	}

	void AssetLoader::ProcessTransform(aiMatrix4x4 nodeMatrix, TransformComponent& localTransform)
	{
		aiVector3D scaling;
		aiVector3D rotation;
		aiVector3D position;
		nodeMatrix.Decompose(scaling, rotation, position);

		localTransform.SetPosition(AssimpGlmHelpers::GetGlmVec(position));
		localTransform.SetScale(AssimpGlmHelpers::GetGlmVec(scaling));
		localTransform.SetRotationEuler(AssimpGlmHelpers::GetGlmVec(rotation), AngleType::Radians);
	}

	void AssetLoader::ProcessNode(GameObject& nodeObject, aiNode* node, const aiScene* assimpScene, std::shared_ptr<Scene> gameScene, const std::string rootName)
	{
		
		
		ProcessTransform(node->mTransformation, nodeObject.GetComponent<TransformComponent>());

		if (node->mNumMeshes > 0)
		{
			aiMesh* aiMesh = assimpScene->mMeshes[node->mMeshes[0]];
			std::shared_ptr<Mesh> mesh = ProcessMesh(aiMesh, assimpScene);
			std::string meshName = node->mParent == nullptr ? rootName : rootName + "_" + aiMesh->mName.C_Str();
			std::string assetIndex = SerializeMesh("../Assets/Meshes/" + meshName + ".pmesh", mesh);
			MeshComponent& component = nodeObject.GetOrAddComponent<MeshComponent>();
			component.SetFilePath(assetIndex);
			component.SetMesh(mesh);

			MaterialInstance& material = component.GetMaterialInstance();
			ProcessTextures(material, aiMesh, assimpScene);
		}
		if(node->mNumMeshes > 1)
		{

			for (unsigned int i = 1; i < node->mNumMeshes; i++)
			{
				aiMesh* aiMesh = assimpScene->mMeshes[node->mMeshes[i]];
				//Logger::Core(LOG_TRACE, "------------------------------");
				std::shared_ptr<Mesh> mesh = ProcessMesh(aiMesh, assimpScene);

				if (!mesh) continue;
				std::string meshName = rootName + "_" + aiMesh->mName.C_Str();
				std::string assetIndex = SerializeMesh("../Assets/Meshes/" + meshName + ".pmesh", mesh);
				GameObject childObject = gameScene->CreateEmptyGameObject(meshName);
				childObject.SetParent(nodeObject);
				MeshComponent& component = childObject.AddComponent<MeshComponent>();
				component.SetFilePath(assetIndex);
				component.SetMesh(mesh);

				MaterialInstance& material = component.GetMaterialInstance();
				ProcessTextures(material, aiMesh, assimpScene);
			}

		}

		// process children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			std::string name = node->mChildren[i]->mName.C_Str();
			GameObject childObject = gameScene->CreateEmptyGameObject(name);
			ProcessNode(childObject, node->mChildren[i], assimpScene, gameScene, rootName);

			nodeObject.AddChild(childObject);
		}
	}


}