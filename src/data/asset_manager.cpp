#include "asset_manager.hpp"

#include <optional>

#include "model_loader.hpp"
#include "../application/engine.hpp"

namespace fs = std::filesystem;

std::optional<pge::ModelView> pge::AssetManager::get_model(std::string_view path)
{
    if (!fs::exists(path))
    {
        return std::nullopt;
    }

    auto key = fs::canonical(path);

    auto iter = m_assets.find(key);

    if (iter == m_assets.end())
    {
        auto model_opt = load_model(path);

        if (!model_opt)
        {
            return std::nullopt;
        }

        auto *model = set_asset(key, model_opt.value());

        for (auto &mesh : model->meshes)
        {
            Engine::renderer->create_buffers(mesh);
        }

        return make_model_view(*model);
    }

    auto model = increment_asset<Model>(iter);

    return make_model_view(*model);
}

pge::Texture* pge::AssetManager::get_texture(std::string_view path, bool flip, TextureWrapMode mode)
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

        auto result = Engine::renderer->create_texture_from_path(path, texture.id, TextureOptions{});

        texture.path = path;

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
        std::visit(overload
        {
            [](Model &model)
            {
                for (auto &mesh : model.meshes)
                {
                    // TODO improve how textures store paths so this wont fail half the time
                    //Engine::asset_manager.free_asset(mesh.material.diffuse.path);
                    Engine::renderer->delete_buffers(mesh);
                }
            },
            [](Texture texture)
            {
                Engine::renderer->delete_texture(texture.id);
            }
        }, iter->second.asset);

        m_assets.erase(iter);
    }
}

std::optional<pge::Model> pge::AssetManager::load_model(std::string_view path)
{
    ModelLoader loader;

    auto model = loader.load(path);

    if (!model)
    {
        return std::nullopt;
    }

    model->id = asset_id++;

    return model;
}