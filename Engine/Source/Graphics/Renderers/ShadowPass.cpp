#include "BsPrecompileHeader.h"
#include "ShadowPass.h"
#include "EngineContext.h"
#include "Resources/AssetLoader.h"
#include "ForwardRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Components/Component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//ToDo: implement shadow map cascades

namespace Pixie
{
	ShadowPass::ShadowPass()
	{
		FrameBufferSpecification frameBufferData;
		frameBufferData.Width = 1024;
		frameBufferData.Height = 1024;

		m_FrameBuffer = FrameBuffer::Create(frameBufferData);
		glEnable(GL_DEPTH_TEST);

		m_Shader = AssetLoader::LoadShader("../Assets/Shaders/SimpleDepthVertex.glsl", "../Assets/Shaders/SimpleDepthFragment.glsl");
	}

	ShadowPass::~ShadowPass()
	{}

	void ShadowPass::Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID, uint32_t prevPassColorID)
	{
		if (m_LightsForcedOff) return;
		// start rendering
		m_Shader->Use();
		m_FrameBuffer->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_FrameBuffer->GetColorAttachmentID());
		// render meshes
		entt::registry& registry = sceneToRender->GetRegistry();
		auto group = registry.group<MeshComponent>(entt::get<TransformComponent>);

		glCullFace(GL_FRONT);
		for (auto entity : group)
		{
			TransformComponent& transform = group.get<TransformComponent>(entity);
			m_Shader->SetUniformMat4("transform", transform.GetObjectToWorldMatrix());

			MeshComponent& mesh = group.get<MeshComponent>(entity);

			mesh.RenderWithoutMaterial(*m_Shader);
		}
		glCullFace(GL_BACK);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_Shader->EndUse();
	}

	uint32_t ShadowPass::GetFrameBufferID() const
	{
		return m_FrameBuffer->GetFrameBufferID();
	}

	uint32_t ShadowPass::GetColorAttatchmentID() const
	{
		return m_FrameBuffer->GetColorAttachmentID();
	}
	uint32_t ShadowPass::GetDepthAttatchmentID() const
	{
		return m_FrameBuffer->GetDepthAttatchmentID();
	}
	bool ShadowPass::IsLit() const
	{
		UniformInfo uniform = m_Shader->GetUniformInfoByName("mainLightPosition");
		if (uniform.IsValid())
			return true;
		else
			return false;
	}
}