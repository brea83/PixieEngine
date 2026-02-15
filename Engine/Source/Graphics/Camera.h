#pragma once

#include <glm/glm.hpp>

namespace Pixie
{
	class Camera
	{
	public:
		Camera(float fov = 45.0f, float aspectRatio = 1280.0f / 720.0f, float nearClip = 0.1f, float farClip = 100.0f);

		//bool BIsSceneViewCam{ false };

		virtual bool HandleMoveWasd(int keyCode, float deltaTime) { return false; }

		virtual bool HandleLookMouse(float xOffset, float yOffset, float deltaTime) { return false; }
		bool Zoom(float amount);
		
		// view matrix is handled by entity's transform component now!
		//virtual glm::mat4 ViewMatrix() const = 0;
		// calculates a projection matrix
		glm::mat4 CalcProjectionMatrix();
		glm::mat4 ProjectionMatrix() const { return m_ProjectionMatrix; }

		glm::mat4 PerspectiveProjection() const;
		glm::mat4 OrthoProjection() const;

		void SetOrthographic(bool value) { m_IsOrthographic = value;  CalcProjectionMatrix(); }
		bool IsOrthographic() const { return m_IsOrthographic; }

		void SetFov(float fov) { m_Fov = fov;  CalcProjectionMatrix(); }
		float GetFov() const { return m_Fov; }

		void SetZoom(float amount) { m_ZoomLevel = amount;  CalcProjectionMatrix(); }
		float GetZoom() const { return m_ZoomLevel; }

		void SetAspectRatio(float aspect) { m_AspectRatio = aspect; CalcProjectionMatrix(); }
		float GetAspectRatio() const { return m_LockAspectRatio ? m_ManualRatio : m_AspectRatio; }
		void LockAspectRatio(bool value = true) { m_LockAspectRatio = value; }
		bool IsAspectRatioLocked() const { return m_LockAspectRatio; }

		void SetNearFar(float nearPlane, float farPlane) { m_Near = nearPlane; m_Far = farPlane;  CalcProjectionMatrix();}
		glm::vec2 GetNearFar() const { return glm::vec2(m_Near, m_Far); }

		bool operator ==(const Camera& other)
		{
			return /* UID or something*/
				m_Fov == other.m_Fov
				&& m_AspectRatio == other.m_AspectRatio
				&& m_Near == other.m_Near
				&& m_Far == other.m_Far;
		}
	protected:
		bool m_IsOrthographic{ false };
		bool m_LockAspectRatio{ false };

		float m_Fov{ 45.0f };
		float m_ZoomLevel{ 1.0f };
		float m_AspectRatio{ 1280.0f / 720.0f };
		float m_ManualRatio{ 1280.0f / 720.0f };

		float m_Near{ 0.01f };
		float m_Far{ 100.0f };

		float m_CameraSpeed{ 10.0f };
		float m_MouseLookSesitivity{ 1.0f };

		glm::mat4 m_ProjectionMatrix{ 1.0f };

		friend class ImGuiLayer;
		friend class DetailsViewPanel;
	};

	
}