#include "mesh.hpp"

#include "image.hpp"

#include <numeric>

namespace msb
{

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<Texture> textures)
    : indices_(indices), textures_(textures)
{
    // layout for Vertex struct with position, normal, tex_coords
    layout_ = {3, 3, 2};
    vertices_.reserve(8 * vertices.size());

    for (auto& v : vertices)
    {
        vertices_.push_back(v.position.x);
        vertices_.push_back(v.position.y);
        vertices_.push_back(v.position.z);
        vertices_.push_back(v.normal.x);
        vertices_.push_back(v.normal.y);
        vertices_.push_back(v.normal.z);
        vertices_.push_back(v.tex_coords.x);
        vertices_.push_back(v.tex_coords.y);
    }

    setupMesh();
}

Mesh::Mesh(std::vector<float> vertices, std::vector<unsigned int> layout,
           std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices_(vertices), layout_(layout), indices_(indices), textures_(textures)
{
    setupMesh();
}

void Mesh::setupMesh()
{
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(),
                 GL_STATIC_DRAW);

    std::vector<unsigned int> offset(layout_.size());
    std::partial_sum(layout_.begin(), layout_.end(), offset.begin());
    auto stride = offset.back();

    if (offset.size() > 1)
    {
        std::rotate(offset.rbegin(), offset.rbegin() + 1, offset.rend());
    }
    offset[0] = 0;

    for (size_t i = 0; i < layout_.size(); ++i)
    {
        glVertexAttribPointer(i, layout_[i], GL_FLOAT, GL_FALSE, stride * sizeof(float),
                              reinterpret_cast<void*>(offset[i] * sizeof(float)));
        glEnableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader) const
{
    unsigned int num_diffuse_maps = 1;
    unsigned int num_specular_maps = 1;
    for (size_t i = 0; i < textures_.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + GLenum(i));
        std::string index;
        std::string name = textures_[i].type;
        if (name == "texture_diffuse")
        {
            index = std::to_string(num_diffuse_maps++);
        }
        else if (name == "texture_specular")
        {
            index = std::to_string(num_specular_maps++);
        }
        shader.setInt(("material." + name + index), i);
        glBindTexture(GL_TEXTURE_2D, textures_[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, GLsizei(indices_.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Texture initTexture(std::string filename, std::string tex_type, unsigned int edge,
                    unsigned int interp, unsigned int cmap)
{
    Texture texture(0, tex_type, filename);

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edge);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edge);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp);

    Image img(filename);

    if (img.data)
    {
        switch (img.nrChannels)
        {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, cmap, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         img.data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, cmap, img.width, img.height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, img.data);
            break;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    return texture;
}

std::ostream& operator<<(std::ostream& os, const Mesh& mesh)
{
    auto verts = mesh.vertices();
    auto layout = mesh.layout();
    auto stride = std::accumulate(layout.begin(), layout.end(), 0);

    os << "Verts: \n";
    for (auto i = 0; i < verts.size(); i += stride)
    {
        if (i > 5 * stride)
        {
            os << "\n...\n";
            break;
        }

        for (auto num_elements : mesh.layout())
        {
            os << "( ";
            for (auto element = 0; element < num_elements; ++element)
            {
                os << verts[i + element] << " ";
            }
            os << ")  ";
        }
        os << "\n";
    }

    stride = 3;
    os << "\nFaces: ";
    for (size_t i = 0; i < mesh.indices().size(); i += stride)
    {
        if (i > 5 * stride)
        {
            os << "\n...\n";
            break;
        }
        os << "\n"
           << mesh.indices()[i] << " " << mesh.indices()[i + 1] << " " << mesh.indices()[i + 2];
    }

    os << "\n";

    return os;
}

} // namespace msb