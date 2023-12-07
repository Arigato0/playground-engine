#version 460 core

out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 view_pos;
uniform float texture_scale;
uniform bool enable_specular;
uniform bool is_spot_light;

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
    vec3 direction;
    float cutoff;
    float outer_cutoff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;
uniform Material material;

float get_attenuation()
{
    float distance = length(light.position - frag_pos);

    return 1.0 / (light.constant + light.linear * distance +
    		    light.quadratic * (distance * distance));
}

vec3 calculate_lighting()
{
    vec3 diffuse_texture = vec3(texture(material.diffuse, text_cord * texture_scale));

    vec3 light_dir = normalize(light.position - frag_pos);

    vec3 norm = normalize(normals);

    float diff = max(dot(norm, light_dir), 0.0);

    vec3 diffuse = light.diffuse * diff * diffuse_texture;
    vec3 ambient =  diffuse_texture * light.ambient;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_texture = vec3(texture(material.specular, text_cord * texture_scale));
    vec3 specular = vec3(0);

    if (enable_specular)
    {
        specular = specular_texture * spec * light.specular;
    }

    if (is_spot_light)
    {
        float theta     = dot(light_dir, normalize(-light.direction));
        float epsilon   = light.cutoff - light.outer_cutoff;
        float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

        diffuse  *= intensity;
        specular *= intensity;
    }

    float attenuation = get_attenuation();

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular + material.emission;
}

void main()
{
    FragColor = vec4(1);

    vec3 lighting = calculate_lighting();

    FragColor *= vec4(lighting, 1.0);
}
