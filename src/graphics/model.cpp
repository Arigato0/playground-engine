#include "model.hpp"

#include "../application/engine.hpp"

void pge::Texture::set(std::string_view path)
{
    Engine::renderer->delete_texture(id);
    Engine::renderer->create_texture(path, id);
}

pge::Texture::Texture(std::string_view path)
{
    Engine::renderer->create_texture(path, id);
    enabled = true;
}

pge::Texture::~Texture()
{
    //::renderer->delete_texture(id);
}
