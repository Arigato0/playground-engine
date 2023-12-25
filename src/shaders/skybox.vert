#version 460

layout(location = 0) in vec3 in_pos;

uniform mat4 view;
uniform mat4 projection;

out vec3 tex_coords;

void main()
{
    vec4 pos = projection * view * vec4(in_pos, 1.0);
    gl_Position = pos.xyww;
    tex_coords = in_pos;
}
