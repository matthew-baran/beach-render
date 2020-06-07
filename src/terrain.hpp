#pragma once

#include "mesh.hpp"

#include <string>
#include <vector>

namespace msb
{

using Geometry = std::pair<std::vector<Vertex>, std::vector<unsigned int>>;

Geometry getPlane(double xsize, double zsize, double step);
Geometry getPlane(double xsize, double zsize, double step, double max_depth);
Geometry getQuad(float xsize, float zsize, float ysize);
Geometry getTerrain(std::string ht_file, std::string norm_file, float xsize, float ysize);

}