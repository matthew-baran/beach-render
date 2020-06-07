#pragma once

#include <string>
#include <vector>

namespace msb
{

unsigned int fillBuffers(std::vector<float> vertices);
unsigned int setHdrTexture(std::string filename);
unsigned int loadCubemap(std::vector<std::string> faces);
std::pair<unsigned int, unsigned int> renderCubeMap(unsigned int hdr_tex_id);
unsigned int renderBrdfQuad();
void renderQuad();

} // namespace msb