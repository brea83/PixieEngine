#include "BsPrecompileHeader.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Pixie
{
	Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
		:   m_Fov(fov), m_AspectRatio(aspectRatio), m_Near(nearClip), m_Far(farClip)
	{
		CalcProjectionMatrix();
	}

	bool Camera::Zoom(float amount)
	{
		m_ZoomLevel -= amount;

		//m_Fov -= amount;

		if (m_ZoomLevel < 0.03f)
		{
			m_ZoomLevel = 0.03f;
		}
		else if (m_ZoomLevel > 2.0f)
		{
			m_ZoomLevel = 2.0f;
		}
		/*if (m_Fov < 1.0f) m_Fov = 1.0f;

		if (m_Fov > 90.0f) m_Fov = 90.0f;*/
		return true;
	}

	glm::mat4 Camera::CalcProjectionMatrix()
	{
		if (m_IsOrthographic)
		{
			m_ProjectionMatrix = OrthoProjection();
		}
		else
		{
			m_ProjectionMatrix = PerspectiveProjection();
		}

		return m_ProjectionMatrix;
	}
	glm::mat4 Camera::PerspectiveProjection() const
	{
		float nonZeroZoom = glm::max(0.001f, m_ZoomLevel);
		float aspectRatio = m_LockAspectRatio ? m_ManualRatio : m_AspectRatio;
		return glm::perspective(glm::radians(m_Fov * nonZeroZoom), aspectRatio, m_Near, m_Far);

		// parameters:
		//float n = m_Near; // the closest your camera ever be
		//float f = m_Far; // the farthest object you ever care
		//float fovx = glm::radians(m_Fov); // horizontal fov in radians
		////float w = 1920, h = 1080; // the size of your viewport (for aspect ratio)

		//// temporaries:
		//float A = 1 / tan(fovx / 2);
		//float B = A * aspectRatio;
		//glm::mat4 Projection =
		//{ 
		//	{A, 0,   0, 0},
		//	{0, B,   0, 0},
		//	{0, 0, (f + n) / (n - f), (2 * n * f) / (n - f) },
		//	{0, 0,  -1, 0} 
		//};
		//return Projection;
	}
	glm::mat4 Camera::OrthoProjection() const
	{
		float nonZeroZoom = glm::max(0.001f, m_ZoomLevel);
		float aspectRatio = m_LockAspectRatio ? m_ManualRatio : m_AspectRatio;
		return glm::ortho(-aspectRatio * nonZeroZoom, aspectRatio * nonZeroZoom, -nonZeroZoom, nonZeroZoom, -10.0f, 20.0f);
	}
}