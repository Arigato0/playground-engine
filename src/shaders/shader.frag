#version 460

out vec4 FragColor;


in vec2 text_cord;

uniform bool enable_color;
uniform vec4 color;
uniform float texture_mix_value = 0.3;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, text_cord), texture(texture2, text_cord), texture_mix_value);

    if (enable_color)
    {
        FragColor *= color;
    }
}