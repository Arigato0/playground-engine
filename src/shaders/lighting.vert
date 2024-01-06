#version 460  core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_tex_cord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

uniform mat4 model;
uniform mat4 mvp;
uniform vec3 view_pos;

out vec2 text_cord;
out vec3 normals;
out vec3 frag_pos;

out vec3 tangent_view_pos;
out vec3 tangent_frag_pos;
out mat3 TBN;

void main()
{
    vec3 T = normalize(vec3(model * vec4(in_tangent,   0.0)));
    vec3 N = normalize(vec3(model * vec4(in_normals,   0.0)));

    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);

    frag_pos = vec3(model * vec4(in_pos, 1.0));

    tangent_view_pos = view_pos * TBN;
    tangent_frag_pos = frag_pos * TBN;

    gl_Position = mvp * vec4(in_pos, 1.0);
    text_cord = in_tex_cord;
    normals = mat3(transpose(inverse(model))) * in_normals;
}