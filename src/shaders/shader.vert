#version 460

layout(location = 0) in vec3 pos;

uniform float x_off;
uniform float y_off;

void main()
{
    gl_Position = vec4(pos.x + x_off, pos.y + y_off, pos.z, 1.0);
}