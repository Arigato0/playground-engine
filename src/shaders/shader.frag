#version 460  core

out vec4 FragColor;

in vec2 text_cord;
in vec3 normals;
in vec3 frag_pos;

uniform bool enable_textures;
uniform bool enable_color;
uniform vec3 object_color;
uniform float texture_mix_value = 0.3;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = vec4(1);

    if (enable_textures)
    {
        FragColor = mix(texture(texture1, text_cord), texture(texture2, text_cord), texture_mix_value);
    }

    if (enable_color)
    {
        FragColor *= vec4(object_color, 1.0);
    }
}