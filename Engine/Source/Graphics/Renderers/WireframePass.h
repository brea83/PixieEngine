#pragma once
#include "RenderPass.h"
#include "Scene/Components/Component.h"

namespace Pixie
{

	class WireframePass : public RenderPass
	{
	public:
		WireframePass();
		~WireframePass();
		void Execute(std::shared_ptr<Scene> sceneToRender, uint32_t prevPassDepthID, uint32_t prevPassColorID) override;
		std::shared_ptr<Shader> GetShader() override { return m_Shader; }

		// this pass just draws on the main frame buffer
		std::shared_ptr<FrameBuffer> GetFrameBuffer() const override { return nullptr; }
		uint32_t GetFrameBufferID() const override { return 0; }
		uint32_t GetColorAttatchmentID() const override { return 0; }
		uint32_t GetDepthAttatchmentID() const override { return 0; }


		bool IsLit() const override { return false; }
		virtual void ForceLightsOff(bool value) override { };

	private:
		std::shared_ptr<Shader> m_Shader{nullptr};
		MaterialInstance m_FallbackMaterial{};
		std::shared_ptr<Mesh> m_CubePrimitive{ nullptr };
		//TODO: add sphere as primitive
		//std::shared_ptr<Mesh> m_SpherePrimitive{ nullptr };
	};
}