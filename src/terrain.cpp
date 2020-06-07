#include "terrain.hpp"

#include "image.hpp"

#include <cmath>
#include <iostream>

namespace msb
{

Geometry getPlane(double xsize, double zsize, double step, double max_depth)
{
	double num_steps_x = std::floor(xsize / step);
	double num_steps_z = std::floor(zsize / step);

	double vert_stride = 3. * 3. * 2.;  // 3 position, 3 normals, 2 uvs
	double tri_stride = 3. * 2.;		 // 3 indices, 2 triangles per step
	std::vector<Vertex> vertices;
	vertices.reserve(static_cast<unsigned int>(num_steps_x * num_steps_z));
	std::vector<unsigned int> indices;
	indices.reserve(static_cast<unsigned int>(3. * 2. * (num_steps_x - 1) * (num_steps_z - 1)));

	for (size_t i = 0; i < num_steps_x; ++i)
	{
		for (size_t j = 0; j < num_steps_z; ++j)
		{
			Vertex vert;
			vert.position.x = static_cast<float>(i * step);
			vert.position.y = float(max_depth * (50. - vert.position.x) / 50.);
			vert.position.z = -static_cast<float>(j * step);

			vert.normal.x = 0;
			vert.normal.y = 1;
			vert.normal.z = 0;

			vert.tex_coords.x = static_cast<float>(float(j) / (num_steps_z - 1));
			vert.tex_coords.y = static_cast<float>(float(i) / (num_steps_x - 1));

			vertices.push_back(vert);

			if (i < num_steps_x - 1 && j < num_steps_z - 1)
			{
				indices.push_back(static_cast<unsigned int>(i * num_steps_z + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * num_steps_z + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * num_steps_z + j + 1));

				indices.push_back(static_cast<unsigned int>(i * num_steps_z + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * num_steps_z + j + 1));
				indices.push_back(static_cast<unsigned int>(i * num_steps_z + j + 1));
			}
		}
	}

	return { vertices, indices };
}

Geometry getPlane(double xsize, double zsize, double step)
{
	return getPlane(xsize, zsize, step, 30);
}

Geometry getQuad(float xsize, float zsize, float ysize)
{
	std::vector<Vertex> vertices(4);

	vertices[0].position = glm::vec3(xsize - 25, -ysize / 2, 0);
	vertices[1].position = glm::vec3(xsize + 25, ysize / 2, 0);
	vertices[2].position = glm::vec3(xsize - 25, -ysize / 2, -zsize);
	vertices[3].position = glm::vec3(xsize + 25, ysize / 2, -zsize);

	vertices[0].tex_coords = glm::vec2(0, 0);
	vertices[1].tex_coords = glm::vec2(15, 0);
	vertices[2].tex_coords = glm::vec2(0, 15);
	vertices[3].tex_coords = glm::vec2(15, 15);

	auto norm = glm::normalize(glm::vec3(ysize / 40, 0, 1));

	for (size_t i = 0; i < 4; ++i)
	{
		vertices[i].normal = norm;
	}

	std::vector<unsigned int> indices = {0, 1, 3, 0, 3, 2};

	return { vertices, indices };
}

Geometry getTerrain(std::string ht_file, std::string norm_file, float xsize, float zsize)
{
	auto ht_img = Image(ht_file);
	auto norm_img = Image(norm_file);

	if (ht_img.data == nullptr || norm_img.data == nullptr)
	{
		std::cout << "Error: Could not load depth/normal images.";
		return { {}, {} };
	}

	std::vector<Vertex> vertices;
	vertices.reserve(ht_img.width * ht_img.height);
	std::vector<unsigned int> indices;
	indices.reserve(3 * 2 * (ht_img.width - 1) * (ht_img.height - 1));

	auto num_px = ht_img.width * ht_img.height;
	auto xstep = xsize / ht_img.width;
	auto zstep = zsize / ht_img.height;

	for (size_t i = 0; i < ht_img.height; ++i)
	{
		for (size_t j = 0; j < ht_img.width; ++j)
		{
			auto ht = 6 * (ht_img.data[ht_img.nrChannels * (i * ht_img.width + j)] / 255.f - 0.5f);

			Vertex vert;
			vert.position.x = xsize * float(j) / (ht_img.width - 1) + 25;
			vert.position.y = ht;
			vert.position.z = zsize * -float(i) / (ht_img.height - 1);

			//std::cout << "Pos: " << vert.position.x << ", " << vert.position.y << ", " << vert.position.z << "\n";

			auto nx = 2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j)] / 255.) - 1;
			auto ny = 2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j) + 1LL] / 255.) - 1;
			auto nz = 2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j) + 2LL] / 255.) - 1;

			// flip to y-is-up
			vert.normal.x = float(nx / xstep);
			vert.normal.y = float(nz);
			vert.normal.z = float(-ny / zstep);
			vert.normal = glm::normalize(vert.normal);

			vert.tex_coords.x = 15.f * j / (ht_img.width - 1.f);
			vert.tex_coords.y = 15.f * i / (ht_img.height - 1.f);

			vertices.push_back(vert);

			if (i < ht_img.height - 1. && j < ht_img.width - 1.)
			{
				indices.push_back(static_cast<unsigned int>(i * ht_img.width + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * ht_img.width + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * ht_img.width + j + 1));

				indices.push_back(static_cast<unsigned int>(i * ht_img.width + j));
				indices.push_back(static_cast<unsigned int>((i + 1) * ht_img.width + j + 1));
				indices.push_back(static_cast<unsigned int>(i * ht_img.width + j + 1));
			}
		}
	}

	return { vertices, indices };
}

}