#include <fstream>
#include "mesh.h"

Mesh::Mesh(const std::string& path)
{
	std::ifstream file(path);

	assert(file.is_open());

	std::vector<GLushort> vIndices, tIndices, nIndices;
	std::vector<glm::vec3> inVertices, inNormals;
	std::vector<glm::vec2> inUvs;

	while (!file.eof())
	{
		std::string prefix;

		file >> prefix >> std::ws;

		if (prefix == "v")
		{
			glm::vec3 v;
			file >> v.x >> v.y >> v.z >> std::ws;
			inVertices.push_back(v);
		}
		else if (prefix == "vt")
		{
			glm::vec2 uv;
			file >> uv.x >> uv.y >> std::ws;
			inUvs.push_back(uv);
		}
		else if (prefix == "vn")
		{
			glm::vec3 v;
			file >> v.x >> v.y >> v.z >> std::ws;
			inNormals.push_back(v);
		}
		else if (prefix == "f")
		{
			for (size_t i = 0; i < 3; i++)
			{
				GLushort v, t, n;
				char c;

				file >> v >> c >> t >> c >> n >> std::ws;
				vIndices.push_back(v);
				tIndices.push_back(t);
				nIndices.push_back(n);
			}
		}
	}

	std::vector <glm::vec3> tVertices, tNormals;
	std::vector <glm::vec2> tUvs;

	for (size_t i = 0; i < vIndices.size(); i++)
	{
		tVertices.push_back(inVertices[vIndices[i] - 1]);

		if (inUvs.size() > 0)
		{
			tUvs.push_back(inUvs[tIndices[i] - 1]);
		}

		if (inNormals.size() > 0)
		{
			tNormals.push_back(inNormals[nIndices[i] - 1]);
		}
	}

	indexVBO(tVertices, tUvs, tNormals);

	file.close();
}

Mesh::~Mesh()
{
}

bool Mesh::getSimilarVertexIndex(Vertex& vertex, std::map<Vertex, GLushort>& vertexToOutIndex, GLushort& result)
{
	std::map<Vertex, GLushort>::iterator it = vertexToOutIndex.find(vertex);

	if (it == vertexToOutIndex.end())
		return false;
	else
	{
		result = it->second;
		return true;
	}
}

void Mesh::indexVBO(std::vector<glm::vec3>& inVertices, std::vector<glm::vec2>& inUvs, std::vector<glm::vec3>& inNormals)
{
	std::map<Vertex, GLushort> vertexToOutIndex;

	for (unsigned int i = 0; i < inVertices.size(); i++)
	{
		Vertex vertex = { inVertices[i], inUvs[i], inNormals[i] };

		GLushort index;
		bool found = getSimilarVertexIndex(vertex, vertexToOutIndex, index);

		if (found)
			indices.push_back(index);
		else
		{
			vertices.push_back(vertex);

			GLushort newIndex = (GLushort)vertices.size() - 1;
			indices.push_back(newIndex);
			vertexToOutIndex[vertex] = newIndex;
		}
	}
}