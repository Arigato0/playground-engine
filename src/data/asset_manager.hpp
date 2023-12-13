#pragma once
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <variant>

#include "../graphics/model.hpp"

namespace pge
{
    using Asset = std::variant<Model, Texture>;

    class AssetManager
    {
        struct AssetData
        {
            Asset asset;
            uint32_t in_use = 0;

            AssetData(Asset &&asset) :
                asset(std::forward<Asset>(asset)),
                in_use(1)
            {}
        };
    public:

        Model* get_model(const std::filesystem::path &path);

    private:
        inline static uint32_t asset_id = 0;
        std::unordered_map<std::filesystem::path, AssetData> m_assets;

        static std::optional<Model> load_model(std::string_view path);
    };
}
