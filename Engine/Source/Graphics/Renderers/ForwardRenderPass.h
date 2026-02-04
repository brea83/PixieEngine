#pragma once
#include "RenderPass.h"
#include "Scene/Components/Component.h"
#include "Scene/Components/MaterialInstance.h"
#include <glm/glm.hpp>


#define MAX_LIGHTS 6

namespace Pixie
{
	class Shader;
	class FrameBuffer;

	class ForwardRenderPass : public RenderPass
	{
	public:
		ForwardRenderPass();
		~ForwardRenderPass();
		// Inherited via RenderPass
		void Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID = 0, uint32_t prevPassColorID = 0) override;
		std::shared_ptr<Shader> GetShader() override { return m_Shader; }
		std::shared_ptr<FrameBuffer> GetFrameBuffer() const override { return nullptr; }
		uint32_t GetFrameBufferID() const override { return 0; }
		uint32_t GetColorAttatchmentID() const override { return 0; }
		uint32_t GetDepthAttatchmentID() const override { return 0; }

		virtual bool IsLit() const override;
		virtual void ForceLightsOff(bool value) override { m_LightsForcedOff = value; };

	private:
		std::shared_ptr<Shader> m_Shader;
		MaterialInstance m_FallbackMaterial{};
		bool m_LightsForcedOff{ false };

		void SendLightsToShader(entt::registry& registry);

	};

}