#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "light.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../common_util/macros.hpp"
#include "../application/log.hpp"

namespace pge
{
    struct Texture
    {
        uint32_t id;
        float scale = 1;
        bool enabled = true;

        Texture() = default;
        void set(std::string_view path);
        Texture(std::string_view path);
        ~Texture();
    };

    struct Material
    {
        float shininess = 32;
        Texture diffuse;
        Texture specular;
        glm::vec3 color {0.0f};
        bool recieve_lighting = true;
    };

    struct Vertex
    {
        glm::vec3 position {};
        glm::vec3 normal   {};
        glm::vec2 coord    {};
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Material material {};
    };

    class Model
    {
    public:

        bool load_model(std::string_view path)
        {
            Assimp::Importer importer;

            auto *scene = importer.ReadFile(path.data(),
                aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph |
                aiProcess_OptimizeMeshes);

            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                return false;
            }

            m_path = path;

            process_node(scene->mRootNode, scene);

            return true;
        }

        std::vector<Mesh> meshes;
    private:
        uint32_t m_id;
        std::filesystem::path m_path;
        std::unordered_map<std::filesystem::path, uint32_t> m_textures;

        void process_node(aiNode *node, const aiScene *scene)
        {
            for (int i = 0; i < node->mNumMeshes; i++)
            {
                auto *mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.emplace_back(process_mesh(mesh, scene));
            }

            for (int i = 0; i < node->mNumChildren; i++)
            {
                process_node(node->mChildren[i], scene);
            }
        }

        std::vector<Vertex> load_mesh_vertices(aiMesh *mesh)
        {
            std::vector<Vertex> output;

            output.reserve(mesh->mNumVertices);

            for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
            {
                auto pos = mesh->mVertices[i];
                auto norm = mesh->mNormals[i];
                auto coord = mesh->mTextureCoords[0][i];

                output.emplace_back
                (
                    Vertex
                    {
                        {EXPAND_VEC3(pos)},
                        {EXPAND_VEC3(norm)},
                        {EXPAND_VEC2(coord)}
                    }
                );
            }

            return output;
        }

        std::vector<uint32_t> load_mesh_indices(aiMesh *mesh)
        {
            std::vector<uint32_t> output;

            //output.reserve(mesh->mNumFaces);

            for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
            {
                auto face = mesh->mFaces[i];

                for (uint32_t j = 0; j < face.mNumIndices; j++)
                {
                    output.push_back(face.mIndices[j]);
                }
            }

            return output;
        }

        Mesh process_mesh(aiMesh *mesh, const aiScene *scene)
        {
            Mesh output;

            output.vertices = load_mesh_vertices(mesh);
            output.indices  = load_mesh_indices(mesh);

            if (mesh->mMaterialIndex >= 0)
            {
                auto *material = scene->mMaterials[mesh->mMaterialIndex];

                output.material.diffuse = load_material(material, aiTextureType_DIFFUSE);
                output.material.specular = load_material(material, aiTextureType_SPECULAR);
            }

            return output;
        }

        Texture load_material(aiMaterial *material, aiTextureType type)
        {
            if (material->GetTextureCount(type) == 0)
            {
                return {};
            }

            aiString path;

            // TODO handle embeded textures
            material->GetTexture(type, 0, &path);

            m_path.replace_filename(path.C_Str()).c_str();

            auto iter = m_textures.find(m_path);

            if (iter != m_textures.end())
            {
                Texture texture;
                texture.id = iter->second;
                return texture;
            }

            Texture texture {m_path.c_str()};

            m_textures.emplace(m_path, texture.id);

            return texture;
        }
    };
}
