#pragma once
#include "Renderer.h"
#include "Graphics/UniformBuffer.h"

namespace Pixie
{
	class TransformComponent;
	class Camera;
	class ForwardRenderer : public Renderer
	{
	public:
		ForwardRenderer(/*EngineContext* engineContext*/);
		// Inherited via Renderer
		void Init() override;
		void BeginFrame(Scene& scene) override;
		void RenderFrame(Scene& scene) override;
		void EndFrame(Scene& scene) override;

		virtual void ForceUnlit(bool value) override;
		virtual void ForceWireFrame(bool value) override;

		virtual std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> GetAllRenderBuffers() override;
	protected:
		std::shared_ptr<Camera> m_ShadowCamera{ nullptr };
		std::shared_ptr<TransformComponent> m_LightTransfrom{ nullptr };
		glm::mat4 m_LightProjection{glm::mat4(1)};
		glm::mat4 m_LightView{glm::mat4(1)};
		UniformBuffer m_LightProjectionUBO;
		UniformBuffer m_CameraBlockUBO;

		bool m_WireFrameOnly{ false };
		struct LightProjUboData
		{
			glm::vec4 mainLightPosition;
			glm::mat4 lightViewMatrix;
			glm::mat4 lightProjectionMatrix;
		};

		struct CameraBlockData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec4 cameraPosition;
		};

		std::vector<glm::vec4> GetFrustumCornersWS(const glm::mat4& projection, const glm::mat4& view);
		glm::vec3 GetFrustumCenter(const std::vector<glm::vec4>& frustumCorners);

	};

}