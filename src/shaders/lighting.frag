#version 460 core

out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 view_pos;
uniform float texture_scale;
uniform bool enable_specular;
uniform bool recieve_lighting;

in vec3 frag_pos;
in vec3 normals;
in vec2 text_cord;

struct Texture
{
    sampler2D sampler;
    float scale;
    bool enabled;
};

struct Material
{
    Texture diffuse;
    Texture specular;
    float shininess;
};

struct Light
{
    bool is_dir;
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

uniform Material material;

#define MAX_LIGHTS 16
uniform Light lights[MAX_LIGHTS];
uniform int light_count;

bool all_is(vec3 vec, float value)
{
    if (vec.x == value && vec.y == value && vec.z == value)
    {
        return true;
    }

    return false;
}

float get_attenuation(Light light)
{
    float distance = length(light.position - frag_pos);

    return 1.0 / (light.constant + light.linear * distance +
    		    light.quadratic * (distance * distance));
}



vec3 create_texture(Texture text)
{
    if (!text.enabled)
    {
        return object_color;
    }

    return vec3(texture(text.sampler, text_cord * texture_scale));
}

struct LightingData
{
    vec3 diffuse_texture;
    vec3 specular_texture;
    vec3 norm;
    vec3 view_dir;
};

vec3 calc_dir_light(Light light, LightingData data)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    float theta   = dot(light_dir, normalize(-light.direction));
    vec3 ambient =  data.diffuse_texture * light.ambient;

    if (theta > light.cutoff)
    {
        float diff = max(dot(data.norm, light_dir), 0.0);

        vec3 diffuse = light.diffuse * diff * data.diffuse_texture;

        vec3 reflect_dir = reflect(-light_dir, data.norm);

        float spec = pow(max(dot(data.view_dir, reflect_dir), 0.0), material.shininess);
        vec3 specular = data.specular_texture * spec * light.specular;

        float epsilon = light.cutoff - light.outer_cutoff;
        float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

        diffuse  *= intensity;
        specular *= intensity;

        float attenuation = get_attenuation(light);

        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation;

        return ambient + diffuse + specular;
    }
    else
    {
        return -ambient;
    }
}

vec3 calculate_lighting(Light light, LightingData data)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(data.norm, light_dir), 0.0);

    vec3 diffuse = light.diffuse * diff * data.diffuse_texture;
    vec3 ambient =  data.diffuse_texture * light.ambient;

    vec3 reflect_dir = reflect(-light_dir, data.norm);

    float spec = pow(max(dot(data.view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = data.specular_texture * spec * light.specular;

    float attenuation = get_attenuation(light);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 result;

    LightingData lighting_data =
    LightingData(
    create_texture(material.diffuse),
    create_texture(material.specular),
    normalize(normals),
    normalize(view_pos - frag_pos));

    if (recieve_lighting)
    {
        for (int i = 0; i < light_count; i++)
        {
            if (lights[i].is_dir)
            {
                result += calc_dir_light(lights[i], lighting_data);
            }
            else
            {
                result += calculate_lighting(lights[i], lighting_data);
            }
        }
    }

    if (any(greaterThan(object_color, vec3(0.0))))
    {
        if (result.x == 0 && result.y == 0 && result.z == 0)
        {
            result = vec3(1);
        }
        result *= object_color;
    }

    FragColor = vec4(result, 1.0);
}
