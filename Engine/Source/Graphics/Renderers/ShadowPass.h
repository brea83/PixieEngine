#pragma once
#include "RenderPass.h"

namespace Pixie
{
	class FrameBuffer;
	class Shader;

	class ShadowPass : public RenderPass
	{
	public:
		ShadowPass();
		virtual ~ShadowPass();

		// Inherited via RenderPass
		void Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID = 0, uint32_t prevPassColorID = 0) override;
		std::shared_ptr<FrameBuffer> GetFrameBuffer() const override { return m_FrameBuffer; }
		std::shared_ptr<Shader> GetShader() override { return m_Shader; }
		uint32_t GetFrameBufferID() const override;
		uint32_t GetColorAttatchmentID() const override;
		uint32_t GetDepthAttatchmentID() const override;

		virtual bool IsLit() const override;
		virtual void ForceLightsOff(bool value) override { m_LightsForcedOff = value; };

	protected:
		std::shared_ptr<FrameBuffer> m_FrameBuffer;
		std::shared_ptr<Shader> m_Shader;
		bool m_LightsForcedOff{ false };
	};
}
