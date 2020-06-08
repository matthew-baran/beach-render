#pragma once

#include "shader.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace msb
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Texture
{
    unsigned int id = NULL;
    std::string type;
    std::string path;

    Texture(unsigned int id, std::string type, std::string path) : id(id), type(type), path(path) {}
};

class Mesh
{
  public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
         std::vector<Texture> textures);
    Mesh(std::vector<float> vertices, std::vector<unsigned int> layout,
         std::vector<unsigned int> indices, std::vector<Texture> textures);

    Mesh() = delete;
    Mesh(const Mesh&) = delete;
    Mesh operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;
    ~Mesh() = default;

    void Draw(const Shader& shader) const;

    std::vector<float> vertices() const { return vertices_; }
    std::vector<unsigned int> indices() const { return indices_; }
    std::vector<unsigned int> layout() const { return layout_; }

  private:
    unsigned int vao_, vbo_, ebo_;

    std::vector<float> vertices_;
    std::vector<unsigned int> layout_;
    std::vector<unsigned int> indices_;
    std::vector<Texture> textures_;

    void setupMesh();
};

Texture initTexture(std::string filename, std::string tex_type, unsigned int edge,
                    unsigned int interp, unsigned int cmap);

std::ostream& operator<<(std::ostream& os, const Mesh& mesh);

} // namespace msb