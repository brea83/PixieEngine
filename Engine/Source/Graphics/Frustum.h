#pragma once
#include <glm/glm.hpp>
#include <memory>
namespace Pixie
{
	class Shader;
	class Frustum
	{
	public:
		Frustum() = default;
		//Frustum(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
		//Frustum(const Frustum&) = default;

		void Recalculate(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
		const std::vector<glm::vec4>& GetCorners() const { return m_Corners; }
		glm::vec3 GetCenter() const;

		static std::vector<glm::vec4> CalcFrustumCornersWS(const glm::mat4& projection, const glm::mat4& view);
		static glm::vec3 CalcFrustumCenter(const std::vector<glm::vec4>& frustumCorners);

		void Render(std::shared_ptr<Shader> shader);
	private:
		std::vector<glm::vec4> m_Corners{};
	};
}