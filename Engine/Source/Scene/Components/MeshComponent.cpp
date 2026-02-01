#include "BsPrecompileHeader.h"
#include "MeshComponent.h"
#include "Resources/AssetLoader.h"
#include "Scene/Components/Transform.h"
#include "Graphics/Primitives/Mesh.h"
#include <glm/glm.hpp>
#include "Assimp/AssimpGlmHelpers.h"
#include <filesystem>
#include <fstream>
#include "Scene/GameObject.h"
#include "EngineContext.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>


namespace Pixie
{
	MeshComponent::MeshComponent()
 : m_Name("Mesh Component"), m_FilePath("")
	{ }

	MeshComponent::MeshComponent(PrimitiveMeshType primitiveMesh)
	: m_Name("Primitive Mesh Component")
	{
		std::shared_ptr<Mesh> mesh = AssetLoader::LoadPrimitive(primitiveMesh);
		if (mesh != nullptr)
		{
			m_Mesh = mesh;

			switch (primitiveMesh)
			{
			case PrimitiveMeshType::Triangle:
				m_FilePath = "PrimitiveMesh_Triangle";
				break;
			case PrimitiveMeshType::Quad:
				m_FilePath = "PrimitiveMesh_Quad";
				break;
			case PrimitiveMeshType::Cube:
				m_FilePath = "PrimitiveMesh_Cube";
				break;
			default:
				break;
			}
		}
		else
		{
			m_FilePath = "";
		}
	}

	MeshComponent::MeshComponent( const std::string& modelFilePath, const std::string& textureFilePath)
		: m_Name("Mesh Component"), m_FilePath(modelFilePath)
	{
		m_MaterialInstance.BaseMapPath = textureFilePath;
		Reload();
	}


	bool MeshComponent::Reload()
	{
		StdPath path(m_FilePath);
		StdPath fileExtension = path.extension();
			GameObject rootObject;
			EngineContext* engine = EngineContext::GetEngine();
			std::shared_ptr<Scene> scene = engine->GetScene();
			entt::registry& registry = scene->GetRegistry();


			auto view = registry.view<MeshComponent>();

			for (auto entity : view)
			{
				MeshComponent& component = view.get<MeshComponent>(entity);
				if (component == *this)
				{
					rootObject = GameObject(entity, scene);
					break;
				}
			}

			if (rootObject.GetScene())
			{
				return AssetLoader::LoadMesh(rootObject, *this, path);
			}
			return false;
	}

	MeshComponent::~MeshComponent()
	{
		Logger::Core(LOG_TRACE, "DELETING {}", m_Name);
	}

	void MeshComponent::OnUpdate()
	{}

	void MeshComponent::Render(Shader& currentShader)
	{
		if (!m_Mesh) return;

		if (m_MaterialInstance.BaseMap != nullptr)
		{
			m_MaterialInstance.BaseMap->Bind(0);
			currentShader.SetUniformInt("Material.ColorTexture", 0);
		}

		if (m_MaterialInstance.NormalMap != nullptr)
		{
			m_MaterialInstance.NormalMap->Bind(1);
			currentShader.SetUniformInt("Material.NormalMap", 1);
			currentShader.SetUniformInt("NormalMap", 1);

			currentShader.SetUniformBool("BUseNormalMap", true);
			currentShader.SetUniformBool("Material.BUseNormalMap", true);
		}
		else
		{
			currentShader.SetUniformBool("BUseNormalMap", false);
			currentShader.SetUniformBool("Material.BUseNormalMap", false);
		}

		if (m_MaterialInstance.MetallicMap != nullptr)
		{
			m_MaterialInstance.MetallicMap->Bind(2);
			currentShader.SetUniformInt("Material.MetallicMap", 2);
			currentShader.SetUniformBool("Material.BUseMetallicMap", true);

			currentShader.SetUniformBool("Material.BMapIsRoughness", m_MaterialInstance.BMapIsRoughness);
			
		}
		else
		{
			currentShader.SetUniformBool("Material.BUseMetalicMap", false);
		}

		currentShader.SetUniformFloat("Material.SpecularPower", m_MaterialInstance.SpecularPower);
		currentShader.SetUniformFloat("Material.Smoothness", m_MaterialInstance.Smoothness);

		m_Mesh->Render(currentShader);

		if (m_MaterialInstance.BaseMap != nullptr)
		{
			m_MaterialInstance.BaseMap->UnBind();
		}
		if (m_MaterialInstance.MetallicMap != nullptr)
		{
			m_MaterialInstance.MetallicMap->UnBind();
		}
	}

	void MeshComponent::RenderWithoutMaterial(Shader& currentShader)
	{
		if (!m_Mesh) return;

		m_Mesh->Render(currentShader);
	}

	//========================================================
	// Circle renderer stuff
	//========================================================
	CircleRendererComponent::CircleRendererComponent()
	{
		MeshResource = AssetLoader::LoadPrimitive(PrimitiveMeshType::Circle);
	}

	CircleRendererComponent::CircleRendererComponent(const glm::vec4& color)
		: Color(color)
	{
		MeshResource = AssetLoader::LoadPrimitive(PrimitiveMeshType::Circle);
	}

	void CircleRendererComponent::Serialize(StreamWriter* stream, const CircleRendererComponent& component)
	{
		stream->WriteRaw(component.Color);
		stream->WriteRaw(component.Radius);
		stream->WriteRaw(component.LineWidth);
		stream->WriteRaw(component.Fade);
	}

	bool CircleRendererComponent::Deserialize(StreamReader* stream, CircleRendererComponent& component)
	{
		stream->ReadRaw(component.Color);
		stream->ReadRaw(component.Radius);
		stream->ReadRaw(component.LineWidth);
		stream->ReadRaw(component.Fade);
		return true;
	}
}
