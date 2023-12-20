#version 460 core

out vec4 FragColor;

in vec2 tex_coords;

uniform sampler2D screen_texture;

void main()
{
    FragColor = texture(screen_texture, tex_coords);
}