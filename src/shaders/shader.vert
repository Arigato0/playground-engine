#version 460

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_cord;

uniform float x_off;
uniform float y_off;

out vec2 text_cord;

void main()
{
    gl_Position = vec4(in_pos.x + x_off, in_pos.y + y_off, in_pos.z, 1.0);
    text_cord = in_tex_cord;
}