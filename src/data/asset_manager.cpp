#include "asset_manager.hpp"

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../common_util/macros.hpp"
#include "../application/engine.hpp"

namespace fs = std::filesystem;

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

            return *pge::Engine::asset_manager.get_texture(root.c_str());
        }

    pge::Model model;
    fs::path root;
};

pge::Model* pge::AssetManager::get_model(std::string_view path)
{
    if (!fs::exists(path))
    {
        return nullptr;
    }

    auto key = fs::canonical(path);

    auto iter = m_assets.find(key);

    if (iter == m_assets.end())
    {
        auto model_opt = load_model(path);

        if (!model_opt)
        {
            return nullptr;
        }

        auto *model = set_asset(key, model_opt.value());

        for (auto &mesh : model->meshes)
        {
            Engine::renderer->create_buffers(mesh);
        }

        return model;
    }

    return increment_asset<Model>(iter);
}

pge::Texture* pge::AssetManager::get_texture(std::string_view path)
{
    if (!fs::exists(path))
    {
        return nullptr;
    }

    auto key = fs::canonical(path);

    auto iter = m_assets.find(key);

    if (iter == m_assets.end())
    {
        Texture texture;

        auto result = Engine::renderer->create_texture(path, texture.id);

        if (result != 0)
        {
            return nullptr;
        }

        return set_asset(key, texture);
    }

    return increment_asset<Texture>(iter);
}

void pge::AssetManager::free_asset(std::string_view path)
{
    auto key = fs::canonical(path);
    auto iter = m_assets.find(key);

    if (iter == m_assets.end())
    {
        return;
    }

    iter->second.in_use--;

    if (iter->second.in_use <= 0)
    {
        m_assets.erase(key);
    }
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