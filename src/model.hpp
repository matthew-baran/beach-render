#pragma once

#include "mesh.hpp"
#include "shader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <vector>

namespace msb
{

class Model
{
  public:
    Model(std::string path) { loadModel(path); }

    Model(Mesh mesh) { meshes.push_back(std::move(mesh)); }

    void Draw(const Shader& shader) const
    {
        for (auto& mesh : meshes)
        {
            mesh.Draw(shader);
        }
    }

  private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> loaded_textures;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                              std::string typeName);
};

} // namespace msb