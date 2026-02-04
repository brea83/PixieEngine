#include "BsPrecompileHeader.h"
#include "ForwardRenderPass.h"
#include "Graphics/Shaders/Shader.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Scene.h"
#include "Scene/Components/Component.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/CameraComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/Texture.h"
#include "Resources/AssetLoader.h"
#include "EngineContext.h"
#include "Scene/GameObject.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Pixie
{
	ForwardRenderPass::ForwardRenderPass()
	{
		m_FallbackMaterial.BaseMapPath = "../Assets/Textures/teal.png";
		m_FallbackMaterial.BaseMap = AssetLoader::LoadTexture(m_FallbackMaterial.BaseMapPath);

		m_Shader = AssetLoader::LoadShader("../Assets/Shaders/VertexShader.glsl", "../Assets/Shaders/FragmentMultiLightLecture.glsl");
		
	}

	ForwardRenderPass::~ForwardRenderPass()
	{ }

	void ForwardRenderPass::Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID, uint32_t prevPassColorID)
	{
		m_Shader->Use();

		if (prevPassDepthID != 0)
		{
			uint32_t slot = 4; // slots 0 through 3 are taken by diffuse, normals, metalic/rough, and specular maps
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, prevPassDepthID);
			m_Shader->SetUniformInt("shadowMap", slot);

			m_Shader->SetUniformBool("bUseShadowMap", true);
		}
		else
		{
			m_Shader->SetUniformBool("bUseShadowMap", false);
		}

		// get scene registry for lighting and renderables
		entt::registry& registry = sceneToRender->GetRegistry();

		// set up light data
		SendLightsToShader(registry);

		// render meshes
		auto group = registry.group<MeshComponent>(entt::get<TransformComponent>);

		for (auto entity : group)
		{
			TransformComponent& transform = group.get<TransformComponent>(entity);
			m_Shader->SetUniformMat4("transform", transform.GetObjectToWorldMatrix());

			MeshComponent& mesh = group.get<MeshComponent>(entity);

			if (!mesh.HasTexture())
			{
				m_FallbackMaterial.BaseMap->Bind(0);
				m_Shader->SetUniformInt("Texture1", 0);
			}

			mesh.Render(*m_Shader);
		}

		m_Shader->EndUse();
	}
	bool ForwardRenderPass::IsLit() const
	{
		UniformInfo uniform = m_Shader->GetUniformInfoByName("BUseLights");
		if (uniform.IsValid())
			return true;
		else
			return false;
	}
	void ForwardRenderPass::SendLightsToShader(entt::registry& registry)
	{
		if (m_LightsForcedOff)
		{
			m_Shader->SetUniformBool("BUseLights", false);
			m_Shader->SetUniformBool("bUseShadowMap", false);
			return;
		}
		auto group = registry.group<LightComponent>(entt::get<TransformComponent>);

		if (!group)
		{
			Logger::Core(LOG_WARNING, "No light components found");
			m_Shader->SetUniformBool("BUseLights", false);
			return;
		}

		m_Shader->SetUniformBool("BUseLights", true);

		int lightTypes[MAX_LIGHTS];
		float lightColors[MAX_LIGHTS * 3];
		float lightPositions[MAX_LIGHTS * 3];
		float lightDirections[MAX_LIGHTS * 3];
		float lightAttenuations[MAX_LIGHTS * 3];
		float innerRadiusCos[MAX_LIGHTS];
		float outerRadiusCos[MAX_LIGHTS];

		int i = 0;
		for (auto entity : group)
		{
			LightComponent& light = group.get<LightComponent>(entity);
			TransformComponent& lightTransform = group.get<TransformComponent>(entity);
			if (!light.Enabled) continue;

			lightTypes[i] = light.Type;

			lightPositions[i * 3 + 0] = lightTransform.GetPosition().x;
			lightPositions[i * 3 + 1] = lightTransform.GetPosition().y;
			lightPositions[i * 3 + 2] = lightTransform.GetPosition().z;

			//glm::vec3 direction = lightTransform.GetRotationEuler(AngleType::Radians);
			//direction = glm::normalize(direction);
			lightDirections[i * 3 + 0] = lightTransform.Forward().x;
			lightDirections[i * 3 + 1] = lightTransform.Forward().y;
			lightDirections[i * 3 + 2] = lightTransform.Forward().z;

			lightColors[i * 3 + 0] = light.Color.r;
			lightColors[i * 3 + 1] = light.Color.g;
			lightColors[i * 3 + 2] = light.Color.b;

			lightAttenuations[i * 3 + 0] = light.Attenuation.x;
			lightAttenuations[i * 3 + 1] = light.Attenuation.y;
			lightAttenuations[i * 3 + 2] = light.Attenuation.z;

			innerRadiusCos[i] = glm::cos(glm::radians(light.InnerRadius));
			outerRadiusCos[i] = glm::cos(glm::radians(light.OuterRadius));

			i++;
		}
		
		int activeLights = i;
		//glUniform1i(glGetUniformLocation(m_Shader->ShaderProgram, "activeLightsCount"), activeLights);
		glUniform1i(glGetUniformLocation(m_Shader->ShaderProgram, "activeLights"), activeLights);
		glUniform3fv(glGetUniformLocation(m_Shader->ShaderProgram, "lightPosition"), activeLights, lightPositions);
		glUniform3fv(glGetUniformLocation(m_Shader->ShaderProgram, "lightDirection"), activeLights, lightDirections);
		glUniform3fv(glGetUniformLocation(m_Shader->ShaderProgram, "lightColor"), activeLights, lightColors);
		glUniform3fv(glGetUniformLocation(m_Shader->ShaderProgram, "lightAttenuation"), activeLights, lightAttenuations);
		glUniform1fv(glGetUniformLocation(m_Shader->ShaderProgram, "innerRadius"), activeLights, innerRadiusCos);
		glUniform1fv(glGetUniformLocation(m_Shader->ShaderProgram, "outerRadius"), activeLights, outerRadiusCos);
		glUniform1iv(glGetUniformLocation(m_Shader->ShaderProgram, "lightTypes"), activeLights, lightTypes);
	}
	
}