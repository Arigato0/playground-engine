#version 460  core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_tex_cord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 text_cord;
out vec3 normals;

void main()
{
    gl_Position = projection * view * model * vec4(in_pos, 1.0);
    text_cord = in_tex_cord;
    normals = in_normals;
}