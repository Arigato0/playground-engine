#pragma once

#include "model_loader.hpp"

#include "stb_image.h"
#include "../common_util/macros.hpp"
#include "../application/engine.hpp"
 #include <glm/gtc/type_ptr.hpp>

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

    model.transform = process_node(model, scene->mRootNode, scene);

    return model;
}

glm::mat4 pge::ModelLoader::process_node(Model &model, aiNode* node, const aiScene* scene)
{
    model.meshes.reserve(scene->mNumMeshes);

	auto node_transform = node->mTransformation;

	glm::mat4 transform {1.0f};

	transform[0] = glm::make_vec4(node_transform[0]);
	transform[1] = glm::make_vec4(node_transform[1]);
	transform[2] = glm::make_vec4(node_transform[2]);
	transform[3] = glm::make_vec4(node_transform[3]);

	model.transform *= transform;

    for (int i = 0; i < node->mNumMeshes; i++)
    {
        auto *mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.emplace_back(process_mesh(mesh, scene));
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        transform *= process_node(model, node->mChildren[i], scene);
    }

	return transform;
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
    return
	{
		.name 	  = mesh->mName.C_Str(),
		.vertices = load_mesh_vertices(mesh),
		.indices  = load_mesh_indices(mesh),
		.material = load_mesh_material(mesh, scene),
	};
}

pge::Material pge::ModelLoader::load_mesh_material(const aiMesh *mesh, const aiScene *scene)
{
	if (mesh->mMaterialIndex < 0)
	{
		return {};
	}

	Material output;

	auto *material = scene->mMaterials[mesh->mMaterialIndex];

	aiColor3D color {};

	material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	material->Get(AI_MATKEY_SHININESS, output.shininess);
	material->Get(AI_MATKEY_OPACITY, output.alpha);
	material->Get(AI_MATKEY_COLOR_SPECULAR, output.specular);

	if (output.alpha < 1.0f)
	{
		output.use_alpha = true;
	}
	if (output.shininess < 1)
	{
		output.shininess = 1;
	}

	output.color = {color.r, color.g, color.b};

	output.diffuse  = load_material(material, aiTextureType_DIFFUSE);

	return output;
}

pge::Texture pge::ModelLoader::load_material(aiMaterial* material, aiTextureType type)
{
    if (material->GetTextureCount(type) == 0)
    {
		return {.enabled = false};
    }

    aiString path;
    // TODO handle embedded textures
    auto result = material->GetTexture(type, 0, &path);

	if (result == aiReturn_FAILURE)
	{
		return {};
	}

    m_path.replace_filename(path.C_Str()).c_str();

    auto *texture = Engine::asset_manager.get_texture(m_path.c_str());

    if (texture == nullptr)
    {
        return {};
    }

    return *texture;
}