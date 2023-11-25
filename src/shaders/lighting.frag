#version 460 core

out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

in vec3 frag_pos;
in vec3 normals;

void main()
{
    FragColor = vec4(1);

    vec3 norm = normalize(normals);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float ambient_strength = 0.2;
    vec3 ambient = ambient_strength * light_color;

    float specular_strength = 0.5;
    int shine = 32;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shine);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * object_color;

    FragColor *= vec4(result, 1.0);
}
