#version 460

out vec4 FragColor;

uniform vec4 color;

void main()
{
    FragColor = vec4(color);
}