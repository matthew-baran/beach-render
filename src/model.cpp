#include "model.hpp"

#include "gl_helpers.hpp"

#include <iostream>

namespace msb
{

void Model::loadModel(std::string path)
{
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/') + 1);

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        auto mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;

        glm::vec3 pos;
        pos.x = mesh->mVertices[i].x;
        pos.y = mesh->mVertices[i].y;
        pos.z = mesh->mVertices[i].z;
        vertex.position = pos;

        glm::vec3 norm;
        norm.x = mesh->mNormals[i].x;
        norm.y = mesh->mNormals[i].y;
        norm.z = mesh->mNormals[i].z;
        vertex.normal = norm;

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 tex;
            tex.x = mesh->mTextureCoords[0][i].x;
            tex.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = tex;
        }
        else
        {
            vertex.tex_coords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps =
            loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps =
            loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                                 std::string type_name)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (size_t j = 0; j < loaded_textures.size(); j++)
        {
            if (loaded_textures[j].path == str.C_Str())
            {
                textures.push_back(loaded_textures[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            auto texture =
                initTexture(directory + str.C_Str(), type_name, GL_REPEAT, GL_LINEAR, GL_SRGB);
            texture.path = str.C_Str();
            textures.push_back(texture);
            loaded_textures.push_back(texture);
        }
    }
    return textures;
}

} // namespace msb