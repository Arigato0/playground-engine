#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <span>
#include <glm/glm.hpp>

#include "../common_util/data_structure.hpp"

namespace pge
{
    struct Texture
    {
        uint32_t id;
        float scale = 1;
        bool enabled = true;
        std::string_view path;
    };

    struct Material
    {
        bool use_alpha = false;
        float shininess = 32;
        float alpha = 1;
        Texture diffuse;
		Texture bump;
		float bump_strength = 1;
        float specular = 0;
        glm::vec3 color {0.0f};
		float emission = 00;
        bool recieve_lighting = true;
		bool cast_shadow = true;
    };

    struct Vertex
    {
        glm::vec3 position 	 {};
        glm::vec3 normal   	 {};
        glm::vec2 coord    	 {};
		glm::vec3 tangent    {};
		glm::vec3 bitangent  {};
    };

    struct Mesh
    {
        uint32_t id = UINT32_MAX;
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Material material {};
    };

    // a read only view of a mesh with owned materials
    struct MeshView
    {
        const uint32_t id;
        const std::string_view name;
        const std::span<const Vertex> vertices;
        const std::span<const uint32_t> indices;
        Material material {};

        MeshView(const Mesh &mesh) :
            id(mesh.id),
            name(mesh.name),
            vertices(mesh.vertices),
            indices(mesh.indices),
            material(mesh.material)
        {}
    };

    template<class T>
    struct ModelBase
    {
        uint32_t id = UINT32_MAX;
        std::vector<T> meshes;
		glm::mat4 transform {};
    };

    using Model = ModelBase<Mesh>;
    using ModelView = ModelBase<MeshView>;

    static ModelView make_model_view(const Model &model)
    {
        ModelView view;

        view.id = model.id;
		view.transform = model.transform;

        util::concat(view.meshes, model.meshes);

        return view;
    }
}
