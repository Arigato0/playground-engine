#version 460 core

out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 view_pos;

in vec3 frag_pos;
in vec3 normals;
in vec2 text_cord;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    vec3 emission;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform Material material;

void main()
{
    FragColor = vec4(1);

    vec3 norm = normalize(normals);
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(norm, light_dir), 0.0);

    vec3 diffuse_texture = vec3(texture(material.diffuse, text_cord));

    vec3 diffuse = light.diffuse * diff * diffuse_texture;

    vec3 ambient =  diffuse_texture * light.ambient;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_texture = vec3(texture(material.specular, text_cord));
    vec3 specular = specular_texture * spec * light.specular;

    vec3 result = ambient + diffuse + specular + material.emission;

    FragColor *= vec4(result, 1.0);
}
