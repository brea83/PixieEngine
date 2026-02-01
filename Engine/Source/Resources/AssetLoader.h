#pragma once
#include "BsPrecompileHeader.h"
#include "Resource.h"
#include "TextResource.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Texture.h"
//#include "Scene/Components/MeshComponent.h"
//#include "Scene/Components/MaterialInstance.h"
#include "Graphics/Primitives/Mesh.h"

#include <assimp/scene.h>
namespace Pixie
{

	class GameObject;
	class Scene;
	class MeshComponent;
	struct MaterialInstance;

	class AssetLoader
	{
	public:
		const static int MimimumAvailableMb{ 2 };
		static bool IsMemoryAvailable(int minimumAvailableMb);

		static std::shared_ptr<TextResource> LoadTextFile(const std::string& filePath);
		static bool ReLoadTextFile(const std::string& filePath);
		static std::shared_ptr<Shader> LoadShader(const std::string& vertPath, const std::string& fragPath);
		static std::shared_ptr<Texture> LoadTexture(const std::string& filePath, TextureType type = TextureType::Diffuse);

		static std::filesystem::path CreatePrefab(GameObject& baseObject);
		static GameObject LoadPrefab(const std::filesystem::path filePath, std::shared_ptr<Scene> scene);
		static bool LoadPrefab(const std::filesystem::path filePath, GameObject& rootObject);

		//static 
		static bool LoadMesh(GameObject& rootObject, MeshComponent& component, const std::filesystem::path filePath);
		static std::shared_ptr<Mesh>LoadMesh(const std::filesystem::path  filePath);
		static std::shared_ptr<Mesh> LoadPrimitive(PrimitiveMeshType primitiveType);
		static std::shared_ptr<Mesh> LoadObj(const std::string& filePath, const std::string& textureFilePath = "");
		//static std::shared_ptr<Mesh> LoadMesh(std::string& objContentsString);
		static bool LoadFbx(const std::filesystem::path filePath, GameObject& rootObject);

		static std::unordered_map<std::string, std::shared_ptr<Resource>>& GetResources() { return s_Resources; }
		static void CleanUp();
	private:
		static std::unordered_map<std::string, std::shared_ptr<Resource>> s_Resources;

		static void ParsePathString(const std::string& inPath, std::string& outPath);
		static std::filesystem::path ParsePathString(const std::filesystem::path& inPath);
		static std::shared_ptr<Texture> LoadTextureParsedPath(const std::string& filePath, TextureType type = TextureType::Diffuse);

		static std::string CheckForSerializedVersion(const std::filesystem::path& filePath);
		static std::string SerializeMesh(const std::string& filePath, std::shared_ptr<Mesh> mesh);
		//static std::string SerializeAssimpSubMesh(const std::string& filePath, const std::string& meshName, std::shared_ptr<Mesh> mesh);
		static std::shared_ptr<Mesh> LoadSerializedMesh(const std::string& filePath);

		//fbx Importing functions 

		static void ProcessNode(GameObject& nodeObject, aiNode* node, const aiScene* assimpScene, std::shared_ptr<Scene> gameScene, const std::string rootName);
		//static GameObject ProcessNodeRecursive(aiNode* node, const aiScene* assimpScene, Scene* gameScene);
		//static std::shared_ptr<Mesh> LoadMesh(aiMesh& mesh, const aiScene& assimpScene);
		static void ProcessTransform(aiMatrix4x4 nodeMatrix, TransformComponent& localTransform);
		//static aiMatrix4x4 CombineTransformsToRoot(aiNode* parentNode, aiNode* childNode);
		static std::shared_ptr<Mesh>  ProcessMesh(aiMesh* mesh, const aiScene* assimpScene);
		static bool ProcessTextures(MaterialInstance& objectMaterial, aiMesh* mesh, const aiScene* assimpScene);

		// returns the first loadable texture in an assimp material of aiTextureType and pixie engine's TextureType
		static std::shared_ptr<Texture> GetTextureFromMaterial(aiMaterial* material, aiTextureType type, TextureType pixieTextureType, std::string& outFilePath);
	};
}

//fbx loading plan
// use similar method as for serializing scene? but it's just the data to construct the fbx again
// first unpack saves meshes in loader with a lookup of their file | node name
//		it will also need to serialize the layout of entities and components used to build the prefab