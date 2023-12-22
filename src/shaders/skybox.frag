#version 460

out vec4 frag_color;

in vec3 tex_coords;

uniform samplerCube skybox_texture;

void main()
{
    frag_color = texture(skybox_texture, tex_coords);
}
