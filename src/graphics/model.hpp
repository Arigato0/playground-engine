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
        bool is_transparent = false;
        float shininess = 32;
        float transparency = 1;
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
        uint32_t id = UINT32_MAX;
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Material material {};
    };

    // a read only view of a mesh with owned materials
    struct MeshView
    {
        const uint32_t &id;
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
    };

    using Model = ModelBase<Mesh>;
    using ModelView = ModelBase<MeshView>;

    static ModelView make_model_view(const Model &model)
    {
        ModelView view;

        view.id = model.id;

        util::concat(view.meshes, model.meshes);

        return view;
    }
}
