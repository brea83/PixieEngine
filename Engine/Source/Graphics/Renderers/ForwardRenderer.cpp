#include "BsPrecompileHeader.h"
#include "ForwardRenderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ForwardRenderPass.h"
#include "CircleRenderPass.h"
#include "ShadowPass.h"
#include "EngineContext.h"
#include "GlfwWrapper.h"
#include "Scene/GameObject.h"
#include "Graphics/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Pixie
{
	ForwardRenderer::ForwardRenderer()
	{ }

	void ForwardRenderer::Init()
	{
		std::unique_ptr<ShadowPass> shadows = std::make_unique<ShadowPass>();
		m_Passes.push_back(std::move(shadows));

		std::unique_ptr<ForwardRenderPass> pass = std::make_unique<ForwardRenderPass>();
		m_Passes.push_back(std::move(pass));

		std::unique_ptr<CircleRenderPass> circlePass = std::make_unique<CircleRenderPass>();
		m_Passes.push_back(std::move(circlePass));

		FrameBufferSpecification frameBufferData;
		glm::vec2 viewportSize = EngineContext::GetEngine()->GetViewportSize();
		frameBufferData.Width = (uint32_t)viewportSize.x;
		frameBufferData.Height = (uint32_t)viewportSize.y;

		m_FrameBuffer = FrameBuffer::Create(frameBufferData);
		glEnable(GL_DEPTH_TEST);

		m_ShadowCamera = std::make_shared<Camera>(45.0f, 1.0f);

		m_LightTransfrom = std::make_shared<TransformComponent>();

		std::vector<uint32_t> shaderIDs;
		for (size_t i = 0; i < m_Passes.size(); i++)
		{
			shaderIDs.push_back(m_Passes[i]->GetShader()->ShaderProgram);
		}

		m_GridShader = AssetLoader::LoadShader("../Assets/Shaders/GridVertex.glsl", "../Assets/Shaders/GridFragment.glsl");
		m_EditorGrid = AssetLoader::LoadMesh("../Assets/Meshes/DebugGrid.obj");

		if (m_GridShader == nullptr || m_EditorGrid == nullptr)
		{
			m_DrawGridEnabled = false;
		}
		else
		{
			shaderIDs.push_back(m_GridShader->ShaderProgram);
			m_DrawGridEnabled = true;
		}

		m_CameraBlockUBO = UniformBuffer(sizeof(CameraBlockData), GL_DYNAMIC_DRAW, 0 /*"CameraBlock", shaderIDs*/);
		m_LightProjectionUBO = UniformBuffer(sizeof(LightProjUboData), GL_DYNAMIC_DRAW, 1/*"LightProjectionBlock", shaderIDs*/);


	}

	void ForwardRenderer::BeginFrame(std::shared_ptr<Scene> scene)
	{
		m_FrameBuffer->Bind();
		glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (scene == nullptr) return;
		// prepare shared uniform buffers
		// -----------------------------------
		// Camera projection UBO
		GameObject cameraEntity = scene->GetActiveCameraGameObject();
		if (!cameraEntity)
		{
			m_bCameraFound = false;
			return; // no valid camera to render
		}

		CameraComponent* camComponent = cameraEntity.TryGetComponent<CameraComponent>();
		if (camComponent == nullptr)
		{
			m_bCameraFound = false;
			Logger::Core(LOG_WARNING, "No Camera in the scene is set to active");
			return;
		}
		Camera& mainCam = camComponent->Cam;
		m_bCameraFound = true;

		// set up rendering camera info
		TransformComponent& transform = cameraEntity.GetComponent<TransformComponent>();
		glm::vec4 camPosition = glm::vec4(transform.GetPosition(), 1);
		glm::mat4 viewMatrix = glm::inverse(transform.GetObjectToWorldMatrix());
		
		glm::mat4 projectionMatrix = mainCam.ProjectionMatrix();

		m_CameraBlockUBO.Bind();
		m_CameraBlockUBO.UpdateMat4(0, viewMatrix);
		m_CameraBlockUBO.UpdateMat4(sizeof(glm::mat4), projectionMatrix);
		m_CameraBlockUBO.UpdateVec4(sizeof(glm::mat4) * 2, camPosition);
		m_CameraBlockUBO.UnBind();

		// main light data needed for shadowmapping is in m_LightProjectionUBO
		GameObject mainLight = scene->GetMainLight();
		glm::mat4 lightSpaceMatrix = glm::mat4();
		glm::vec4 hypotheticalLightPos{ 1.0f };


		if (mainLight)
		{
			//TEMP 
			// TODO: move camera frustrum fitting somewhere that makes more sense
			//fake projection matrix with different near/far for better shadows
			m_ShadowCamera->SetAspectRatio(mainCam.GetAspectRatio());
			m_ShadowCamera->SetFov(mainCam.GetFov());
			m_ShadowCamera->SetZoom(mainCam.GetZoom());
			m_ShadowCamera->SetOrthographic(mainCam.IsOrthographic());
			m_ShadowCamera->SetNearFar(0.01f, 35.0f);
			glm::mat4 shadowProjection = m_ShadowCamera->ProjectionMatrix();
			std::vector< glm::vec4> frustumWS = GetFrustumCornersWS(shadowProjection, viewMatrix);
			glm::vec3 frustumCenter = GetFrustumCenter(frustumWS);


			glm::vec2 viewportSize = EngineContext::GetEngine()->GetViewportSize();
			float aspectRatio = viewportSize.x / viewportSize.y;
			// light direction is used as position with an ortho projection
			LightComponent& light = mainLight.GetComponent<LightComponent>();
			TransformComponent lightTransform = mainLight.GetTransform();

			glm::vec3 forward = lightTransform.Forward();
			glm::vec3 forwardDegrees = glm::degrees(forward);

			hypotheticalLightPos = glm::vec4(-1.0f * forward, 1);
			m_LightTransfrom->SetPosition(hypotheticalLightPos);
			m_LightTransfrom->SetRotationEuler(lightTransform.GetRotationEuler());

			m_LightView = glm::inverse(m_LightTransfrom->GetObjectToWorldMatrix());
			

			glm::vec3 minPointLS{ std::numeric_limits<float>::max() };
			glm::vec3 maxPointLS{ std::numeric_limits<float>::lowest() };

			for (const glm::vec4& point : frustumWS)
			{
				const glm::vec4 pointLS = m_LightView * point;

				minPointLS.x = std::min(minPointLS.x, pointLS.x);
				minPointLS.y = std::min(minPointLS.y, pointLS.y);
				minPointLS.z = std::min(minPointLS.z, pointLS.z);

				maxPointLS.x = std::max(maxPointLS.x, pointLS.x);
				maxPointLS.y = std::max(maxPointLS.y, pointLS.y);
				maxPointLS.z = std::max(maxPointLS.z, pointLS.z);
			}

			// zMult needs to be tuned to scene so TODO: parameterize this so it can be changed in editor
			constexpr float zMult = 10.0f;
			if (minPointLS.z < 0)
				minPointLS.z *= zMult;
			else
				minPointLS.z /= zMult;

			if (maxPointLS.z < 0)
				maxPointLS.z /= zMult;
			else
				maxPointLS.z *= zMult;


			m_LightProjection = glm::ortho<float>(minPointLS.x, maxPointLS.x, minPointLS.y, maxPointLS.y, minPointLS.z, maxPointLS.z);

			lightSpaceMatrix = m_LightProjection * m_LightView;

			m_LightProjectionUBO.Bind();
			m_LightProjectionUBO.UpdateVec4(0, hypotheticalLightPos);
			m_LightProjectionUBO.UpdateMat4(16, m_LightView);
			m_LightProjectionUBO.UpdateMat4(80, m_LightProjection);
			m_LightProjectionUBO.UnBind();

		}

		

	}

	void ForwardRenderer::RenderFrame(std::shared_ptr<Scene> scene)
	{
		if (scene == nullptr) return;

		uint32_t prevPassDepth{ 0 };
		uint32_t prevPassColor{ 0 };

		if (!m_bCameraFound) return;

		if (m_WireFrameOnly)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT, GL_FILL);
		}

		for (size_t i = 0; i < m_Passes.size(); i++)
		{

			m_Passes[i]->Execute(scene, prevPassDepth, prevPassColor);

			prevPassDepth = m_Passes[i]->GetDepthAttatchmentID();
			prevPassColor = m_Passes[i]->GetColorAttatchmentID();
			
			std::shared_ptr<FrameBuffer> passBuffer = m_Passes[i]->GetFrameBuffer();
			if (passBuffer != nullptr)
				m_FrameBuffer->Bind();
		}
		
		// post processing?

		if (m_DrawGridEnabled && m_bCameraFound)
		{
			m_GridShader->Use();
		
			glm::mat4 transformMatrix = glm::mat4(1.0f); // send an identiy matrix, grid mesh is always at origin
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
			transformMatrix = transformMatrix * scale;
			m_GridShader->SetUniformMat4("transform", transformMatrix);
			
			m_EditorGrid->Render(*m_GridShader);
			m_GridShader->EndUse();
		}
		
	}

	

	void ForwardRenderer::EndFrame(std::shared_ptr<Scene> scene)
	{
		m_FrameBuffer->UnBind();
	}

	void ForwardRenderer::ForceUnlit(bool value)
	{
		for (size_t i = 0; i < m_Passes.size(); i++)
		{
			m_Passes[i]->ForceLightsOff(value);
		}
	}

	void ForwardRenderer::ForceWireFrame(bool value)
	{
		m_WireFrameOnly = value;
	}

	std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> ForwardRenderer::GetAllRenderBuffers()
	{
		std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> renderBuffers;
		for (size_t i = 0; i < m_Passes.size(); i++)
		{
			std::shared_ptr<FrameBuffer> passBuffer = m_Passes[i]->GetFrameBuffer();
			if (passBuffer == nullptr) continue;

			std::shared_ptr<Shader> shader = m_Passes[i]->GetShader();
			std::string name = shader->GetName();

			if (renderBuffers.find(name) != renderBuffers.end())
			{
				name += " ";
				name += std::to_string(i);
			}

			renderBuffers[name] = passBuffer;
		}

		renderBuffers["Main Buffer"] = m_FrameBuffer;
		return renderBuffers;
	}

	std::vector<glm::vec4> ForwardRenderer::GetFrustumCornersWS(const glm::mat4& projection, const glm::mat4& view)
	{
		const auto inverse = glm::inverse(projection * view);

		std::vector<glm::vec4> frustumCorners;
		for (int x = 0; x < 2; x++)
		{
			for (int y = 0; y < 2; y++)
			{
				for (int z = 0; z < 2; z++)
				{
					glm::vec4 point
					{
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f
					};

					point = inverse * point;

					frustumCorners.push_back(point / point.w);
				}
			}
		}

		return frustumCorners;
	}
	glm::vec3 ForwardRenderer::GetFrustumCenter(const std::vector<glm::vec4>& frustumCorners)
	{
		glm::vec3 center{ 0.0f };
		for (const auto& point : frustumCorners)
		{
			center += glm::vec3(point);
		}

		return center /= (float)frustumCorners.size();
	}
}