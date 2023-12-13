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

        Model*   get_model(std::string_view path);
        Texture* get_texture(std::string_view path);

        void free_asset(std::string_view path);

    private:
        using AssetTable = std::unordered_map<std::filesystem::path, AssetData>;
        inline static uint32_t asset_id = 0;
        AssetTable m_assets;

        static std::optional<Model> load_model(std::string_view path);

        template<class T>
        T* set_asset(std::filesystem::path &key, T &asset)
        {
            const auto &[iter, inserted] = m_assets.emplace(key, asset);

            if (!inserted)
            {
                return nullptr;
            }

            return &std::get<T>(iter->second.asset);
        }

        template<class T>
        T* increment_asset(AssetTable::iterator iter)
        {
            auto &data = iter->second;

            data.in_use++;

            return &std::get<T>(data.asset);
        }
    };
}
