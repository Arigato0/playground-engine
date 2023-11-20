#version 460

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_cord;

uniform mat4 transform;

out vec2 text_cord;

void main()
{
    gl_Position = transform * vec4(in_pos, 1.0);
    text_cord = in_tex_cord;
}