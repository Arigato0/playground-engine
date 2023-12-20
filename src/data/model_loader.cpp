#pragma once

#include "model_loader.hpp"

#include "stb_image.h"
#include "../common_util/macros.hpp"
#include "../application/engine.hpp"

std::optional<pge::Model> pge::ModelLoader::load(std::string_view path, int flags)
{
    Assimp::Importer importer;

    auto *scene = importer.ReadFile(path.data(), flags);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return std::nullopt;
    }

    m_path = path;

    Model model;

    process_node(model, scene->mRootNode, scene);

    return model;
}

void pge::ModelLoader::process_node(Model &model, aiNode* node, const aiScene* scene)
{
    model.meshes.reserve(scene->mNumMeshes);

    for (int i = 0; i < node->mNumMeshes; i++)
    {
        auto *mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.emplace_back(process_mesh(mesh, scene));
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        process_node(model, node->mChildren[i], scene);
    }
}

std::vector<pge::Vertex> pge::ModelLoader::load_mesh_vertices(aiMesh* mesh)
{
    std::vector<Vertex> output;

    output.reserve(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        auto pos = mesh->mVertices[i];
        glm::vec3 norm {};
        glm::vec2 coord {};

        if (mesh->HasNormals())
        {
            auto vec = mesh->mNormals[i];
            norm = {EXPAND_VEC3(vec)};
        }
        if (mesh->mTextureCoords[0])
        {
            coord = {EXPAND_VEC2(mesh->mTextureCoords[0][i])};
        }

        output.emplace_back
        (
            Vertex
            {
                {EXPAND_VEC3(pos)},
                norm,
                coord
            }
        );
    }

    return output;
}

std::vector<uint32_t> pge::ModelLoader::load_mesh_indices(aiMesh* mesh)
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

pge::Mesh pge::ModelLoader::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    Mesh output;

    output.vertices = load_mesh_vertices(mesh);
    output.indices  = load_mesh_indices(mesh);

    output.name = mesh->mName.C_Str();

    if (mesh->mMaterialIndex >= 0)
    {
        auto *material = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D color {};

        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        material->Get(AI_MATKEY_SHININESS, output.material.shininess);
        material->Get(AI_MATKEY_OPACITY, output.material.alpha);

        if (output.material.alpha < 1.0f)
        {
            output.material.use_alpha = true;
        }
        if (output.material.shininess < 1)
        {
            output.material.shininess = 1;
        }

        output.material.color = {color.r, color.g, color.b};

        output.material.diffuse  = load_material(material, aiTextureType_DIFFUSE);
        output.material.specular = load_material(material, aiTextureType_SPECULAR);
    }

    return output;
}

pge::Texture pge::ModelLoader::load_material(aiMaterial* material, aiTextureType type)
{
    if (material->GetTextureCount(type) == 0)
    {
        return {};
    }

    aiString path;
    // TODO handle embeded textures
    material->GetTexture(type, 0, &path);

    m_path.replace_filename(path.C_Str()).c_str();

    auto *texture = Engine::asset_manager.get_texture(m_path.c_str());

    if (texture == nullptr)
    {
        return {};
    }

    return *texture;
}
