#pragma once
#include <vector>
#include <memory>
#include "Scene/Scene.h"
#include "RenderPass.h"
#include "Graphics/FrameBuffer.h"

namespace Pixie
{
	class EngineContext;
	class Shader;
	class Mesh;

	class Renderer
	{
	public:
		virtual void Init() = 0;
		virtual void BeginFrame(std::shared_ptr<Scene> scene) = 0;
		virtual void RenderFrame(std::shared_ptr<Scene> scene) = 0;
		virtual void EndFrame(std::shared_ptr<Scene> scene) = 0;

		std::shared_ptr<FrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }
		virtual uint32_t GetFrameBufferID() { return m_FrameBuffer->GetColorAttachmentID(); }

		std::shared_ptr<Shader> GetGridShader() { return m_GridShader; }
		void EnableGridShader(bool value) { m_DrawGridEnabled = value; }
		bool IsGridShaderEnabled() { return m_DrawGridEnabled; }

		virtual void ForceUnlit(bool value) = 0;
		virtual void ForceWireFrame(bool value) = 0;

		virtual std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> GetAllRenderBuffers() = 0;
	protected:
		EngineContext* m_Engine{ nullptr };
		std::vector<std::unique_ptr<RenderPass>> m_Passes;

		std::shared_ptr<FrameBuffer> m_FrameBuffer{ nullptr };

		bool m_bCameraFound{ false };
		bool m_DrawGridEnabled{ false };
		std::shared_ptr<Shader> m_GridShader{ nullptr };
		std::shared_ptr<Mesh> m_EditorGrid{ nullptr };

	private:
		friend class WireframePass;

	};

}


// call this line to draw in wireframe polygons.
//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);