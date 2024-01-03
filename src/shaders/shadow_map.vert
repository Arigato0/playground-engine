#version 460  core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_tex_cord;

uniform mat4 model;
uniform mat4 light_space;

void main()
{
    gl_Position = light_space * model * vec4(in_pos, 1.0);
}