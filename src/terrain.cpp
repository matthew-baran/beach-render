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

    double vert_stride = 3. * 3. * 2.; // 3 position, 3 normals, 2 uvs
    double tri_stride = 3. * 2.;       // 3 indices, 2 triangles per step
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

    return {vertices, indices};
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

    return {vertices, indices};
}

GeometryF getTerrain(std::string ht_file, std::string norm_file, float xsize, float zsize)
{
    auto ht_img = Image(ht_file);
    auto norm_img = Image(norm_file);

    if (ht_img.data == nullptr || norm_img.data == nullptr)
    {
        std::cout << "Error: Could not load depth/normal images.";
        return {{}, {}};
    }

    auto stride = 3 + 3 + 2 + 3; // position, normal, tex_coord, tangent

    std::vector<float> vertices;
    vertices.reserve(ht_img.width * ht_img.height * stride);

    std::vector<glm::vec3> positions;
    positions.reserve(ht_img.width * ht_img.height);

    std::vector<glm::vec2> uv;
    uv.reserve(ht_img.width * ht_img.height);

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

            // position
            auto pos = glm::vec3(xsize * float(j) / (ht_img.width - 1) + 25, ht,
                                 zsize * -float(i) / (ht_img.height - 1));

            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);

            positions.push_back(pos);

            // normal
            auto nx =
                2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j)] / 255.) - 1;
            auto ny =
                2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j) + 1] / 255.) - 1;
            auto nz =
                2 * (norm_img.data[norm_img.nrChannels * (i * norm_img.width + j) + 2] / 255.) - 1;

            // normalize and flip to y-is-up, and swap x/z
            auto normal = glm::normalize(glm::vec3(ny / xstep, nz, -nx / zstep));

            vertices.push_back(float(normal.x));
            vertices.push_back(float(normal.y));
            vertices.push_back(float(normal.z));

            // tex_coords
            auto uv_val =
                glm::vec2(15.f * j / (ht_img.width - 1.f), 15.f * i / (ht_img.height - 1.f));

            vertices.push_back(uv_val.x);
            vertices.push_back(uv_val.y);

            uv.push_back(uv_val);

            // zero fill for tangent
            vertices.push_back(0.);
            vertices.push_back(0.);
            vertices.push_back(0.);

            // triangles
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

    // compute vertex tangents
    for (auto i = 0; i < indices.size(); i += 3)
    {
        auto& p1 = positions[indices[i]];
        auto& p2 = positions[indices[i + 1]];
        auto& p3 = positions[indices[i + 2]];
        auto& uv1 = uv[indices[i]];
        auto& uv2 = uv[indices[i + 1]];
        auto& uv3 = uv[indices[i + 2]];

        auto edge1 = p2 - p1;
        auto edge2 = p3 - p1;
        auto delta_uv1 = uv2 - uv1;
        auto delta_uv2 = uv3 - uv1;

        auto f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

        glm::vec3 tangent;
        tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
        tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
        tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

        auto offset = indices[i] * stride + 8;
        vertices[offset] += tangent.x;
        vertices[offset + 1] += tangent.y;
        vertices[offset + 2] += tangent.z;
    }

    return {vertices, indices};
}

} // namespace msb