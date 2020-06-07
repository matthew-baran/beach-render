#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace geometry
{
	using Mesh = std::pair<std::vector<float>, std::vector<unsigned int>>;
}

geometry::Mesh makeSkybox();
geometry::Mesh makeBox();
geometry::Mesh makeBox(bool add_colors);
geometry::Mesh makeBox2();
geometry::Mesh makeTriangle(float base, float x_center, unsigned int start_idx);
geometry::Mesh makeTriangle2(float base, std::pair<float, float> center, unsigned int start_idx, bool add_colors);
geometry::Mesh makeCube();
geometry::Mesh makeCubeNormals();
geometry::Mesh makeCubeTexture();
std::vector<glm::vec3> getCubePositions();
std::vector<glm::vec3> getLightPositions();