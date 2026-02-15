#pragma once
#include "Mesh.h"

namespace Pixie
{
	class Cube : public Mesh
	{
	public:
		//Cube();
		Cube( /*unsigned int uid, */bool useMinimalVerticies = false, const std::string& name = "Cube");
		//Cube(std::vector<Vertex>& newVerticies, std::vector<glm::uvec3>& newTriangleIndices);
		~Cube();

		// Inherited via Renderable
		void Render(Shader& currentShader) override;
	protected:
		void Init() override;
	private:
		friend class WireframePass;
	};

}