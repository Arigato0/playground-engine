#include "asset_manager.hpp"

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../common_util/macros.hpp"
#include "../application/engine.hpp"

struct ModelLoader
{
    bool load(std::string_view path)
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

        root = path;

        process_node(scene->mRootNode, scene);

        return true;
    }

    void process_node(aiNode *node, const aiScene *scene)
    {
        model.meshes.reserve(scene->mNumMeshes);

        for (int i = 0; i < node->mNumMeshes; i++)
        {
            auto *mesh = scene->mMeshes[node->mMeshes[i]];
            model.meshes.emplace_back(process_mesh(mesh, scene));
        }

        for (int i = 0; i < node->mNumChildren; i++)
        {
            process_node(node->mChildren[i], scene);
        }
    }

    std::vector<pge::Vertex> load_mesh_vertices(aiMesh *mesh)
        {
            std::vector<pge::Vertex> output;

            output.reserve(mesh->mNumVertices);

            for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
            {
                auto pos = mesh->mVertices[i];
                auto norm = mesh->mNormals[i];

                glm::vec2 coord {};

                if (mesh->mTextureCoords[0])
                {
                    coord = {EXPAND_VEC2(mesh->mTextureCoords[0][i])};
                }

                output.emplace_back
                (
                    pge::Vertex
                    {
                        {EXPAND_VEC3(pos)},
                        {EXPAND_VEC3(norm)},
                        coord
                    }
                );
            }

            return output;
        }

        std::vector<uint32_t> load_mesh_indices(aiMesh *mesh)
        {
            std::vector<uint32_t> output;

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

        pge::Mesh process_mesh(aiMesh *mesh, const aiScene *scene)
        {
            pge::Mesh output;

            output.vertices = load_mesh_vertices(mesh);
            output.indices  = load_mesh_indices(mesh);

            output.name = mesh->mName.C_Str();

            if (mesh->mMaterialIndex >= 0)
            {
                auto *material = scene->mMaterials[mesh->mMaterialIndex];

                aiColor3D color {};
                float shininess;
                material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                material->Get(AI_MATKEY_SHININESS, shininess);

                output.material.color = {color.r, color.g, color.b};
                output.material.shininess = shininess;

                output.material.diffuse = load_material(material, aiTextureType_DIFFUSE);
                output.material.specular = load_material(material, aiTextureType_SPECULAR);
            }

            return output;
        }

        pge::Texture load_material(aiMaterial *material, aiTextureType type)
        {
            if (material->GetTextureCount(type) == 0)
            {
                return {};
            }

            aiString path;

            // TODO handle embeded textures
            material->GetTexture(type, 0, &path);

            root.replace_filename(path.C_Str()).c_str();

            auto iter = m_textures.find(root);

            if (iter != m_textures.end())
            {
                pge::Texture texture;
                texture.id = iter->second;
                return texture;
            }

            pge::Texture texture {root.c_str()};

            m_textures.emplace(root, texture.id);

            return texture;
        }

    pge::Model model;
    std::filesystem::path root;
    std::unordered_map<std::filesystem::path, uint32_t> m_textures;
};

pge::Model* pge::AssetManager::get_model(const std::filesystem::path& path)
{
    if (!exists(path))
    {
        return nullptr;
    }

    const auto key = canonical(path);

    auto iter = m_assets.find(key);

    if (iter == m_assets.end())
    {
        auto model_opt = load_model(path.c_str());

        if (!model_opt)
        {
            return nullptr;
        }

        const auto &[iter, inserted] = m_assets.emplace(key, model_opt.value());

        if (!inserted)
        {
            return nullptr;
        }

        auto &model = std::get<Model>(iter->second.asset);

        for (auto &mesh : model.meshes)
        {
            Engine::renderer->create_buffers(mesh);
        }

        return &model;
    }

    auto &data = iter->second;

    data.in_use++;

    return &std::get<Model>(data.asset);
}

std::optional<pge::Model> pge::AssetManager::load_model(std::string_view path)
{
    ModelLoader loader;

    auto ok = loader.load(path);

    if (!ok)
    {
        return std::nullopt;
    }

    loader.model.id = asset_id++;

    return loader.model;
}
