#include "BsPrecompileHeader.h"
#include "Frustum.h"
#include "Shaders/Shader.h"

namespace Pixie
{

	//Frustum::Frustum(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
	//{
	//	m_Corners = CalcFrustumCornersWS(projectionMatrix, viewMatrix);
	//}

	void Frustum::Recalculate(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
	{
		m_Corners = CalcFrustumCornersWS(projectionMatrix, viewMatrix);
	}

	std::vector<glm::vec4> Frustum::CalcFrustumCornersWS(const glm::mat4& projection, const glm::mat4& view)
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
    
	glm::vec3 Frustum::CalcFrustumCenter(const std::vector<glm::vec4>& frustumCorners)
    {
		glm::vec3 center{ 0.0f };
		for (const auto& point : frustumCorners)
		{
			center += glm::vec3(point);
		}

		return center /= (float)frustumCorners.size();
    }
    
	void Frustum::Render(std::shared_ptr<Shader> shader)
    {}
}