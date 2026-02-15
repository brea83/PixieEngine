#include "BsPrecompileHeader.h"
#include "Cube.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Pixie
{
	static int s_NumCubes = 1;
	Cube::Cube(bool useMinimalVerticies, const std::string& name)
		: Mesh((name + " " + std::to_string(s_NumCubes++)))
	{
		if (useMinimalVerticies)
		{
			m_Vertices = {
				// back face
				Vertex{{-0.5f,  0.5f, -0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }}, // top left
				Vertex{{ 0.5f,  0.5f, -0.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f }}, // top right
				Vertex{{-0.5f, -0.5f, -0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f }}, // bottom left
				Vertex{{ 0.5f, -0.5f, -0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f }}, // bottom right

				// front face
				Vertex{{-0.5f,  0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }}, // top left
				Vertex{{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f }}, // top right
				Vertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f }}, // bottom left
				Vertex{{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f }}, // bottom right
			};

			m_Indices = {  //idicies of the vertexies list to use as points of a triangle
				// back face
				0, 1, 2, 1, 2, 3,
				// front face
				4, 5, 6 , 5, 6, 7,
				// top face
				0, 1, 5 , 1, 5, 4,
				// bottom fac
				2, 3, 7 , 2, 7, 6,
				// left face
				0, 4, 2 , 4, 2, 6,
				//right face
				1, 5, 7 , 1, 3, 7,
			};
		}
		else
		{
			m_Vertices = {
				// front face
				Vertex{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f }},
				Vertex{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f }},
				Vertex{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f }},
				Vertex{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f }},
				// back face
				Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}, { 1.0f, 0.0f }},
				Vertex{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}, { 1.0f, 1.0f }},
				Vertex{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}, { 0.0f, 1.0f }},
				Vertex{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}, { 0.0f, 0.0f }},

				//bottom face 
				Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, { 0.0f, 1.0f }},
				Vertex{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, { 1.0f, 1.0f }},
				Vertex{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, { 1.0f, 0.0f }},
				Vertex{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, { 0.0f, 0.0f }},

				//top face
				Vertex{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f }},
				Vertex{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f }},
				Vertex{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f }},
				Vertex{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f }},

				//left face
				Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
				Vertex{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
				Vertex{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, { 0.0f, 1.0f }},
				Vertex{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, { 1.0f, 1.0f }},

				//right face
				Vertex{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }},
				Vertex{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }},
				Vertex{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }},
				Vertex{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }},
			};

			m_Indices = {
				// Front face
				0, 2, 1, 2, 0, 3,
				// Back face
				4, 6, 5, 6, 4, 7,
				// Bottom face
				8, 10, 9, 10, 8, 11,
				// Top face
				12, 14, 13, 14, 12, 15,
				// Left face
				16, 18, 17 , 18, 16, 19 ,
				// Right face
				20, 22, 21, 22, 20, 23
			};
		}

		Init();
	}

	void Cube::Init()
	{
		// generate vert array and vert buffer
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);


		//bind vert array first, then bind the buffer and tell it how ot traverse the array.
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

		// set up element buffer object, which gets saved on the VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

		//position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		glEnableVertexAttribArray(0);
		//vertex color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
		glEnableVertexAttribArray(1);
		// vertex normals
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		//vertex uv
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV1));
		glEnableVertexAttribArray(3);

		// unbind so that other objects can be set up
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Cube::~Cube()
	{
		if (VBO) glDeleteBuffers(1, &VBO);
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (EBO) glDeleteBuffers(1, &EBO);
	}

	void Cube::Render(Shader& currentShader)
	{
		//draw
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}