#version 460 core

out vec4 frag_color;

uniform vec3 view_pos;
uniform float texture_scale;
uniform bool recieve_lighting;
uniform bool visualize_depth;

in vec3 frag_pos;
in vec3 normals;
in vec2 text_cord;

struct Texture
{
    sampler2D sampler;
    bool enabled;
};

struct Material
{
    Texture diffuse;
    float specular;
    float shininess;
    float transparency;
    vec3 color;
};

struct Light
{
    bool is_active;
    bool is_spot;
    vec3 position;
    vec3 direction;
    float cutoff;
    float outer_cutoff;

    vec3 color;
    float ambient;
    float diffuse;
    float specular;

    float power;

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
        return material.color;
    }

    return vec3(texture(text.sampler, text_cord * texture_scale));
}

struct LightingData
{
    vec3 diffuse;
    float specular;
    vec3 norm;
    vec3 view_dir;
};

vec3 calculate_lighting(Light light, LightingData data)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(data.norm, light_dir), 0.0);

    vec3 diffuse = light.color * light.diffuse * diff * data.diffuse * light.power;
    vec3 ambient =  light.color * data.diffuse * light.ambient * light.power;

    vec3 reflect_dir = reflect(-light_dir, data.norm);

    float spec = pow(max(dot(data.view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.color * data.specular * spec * light.specular * light.power;

    if (light.is_spot)
    {
        float epsilon = light.cutoff - light.outer_cutoff;
        float theta   = dot(light_dir, normalize(-light.direction));
        float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

        diffuse  *= intensity;
        specular *= intensity;
    }

    float attenuation = get_attenuation(light);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

uniform float near;
uniform float far;

vec4 calculate_depth()
{
    float z = gl_FragCoord.z * 2.0 - 1.0;
    float linear_depth = (2.0 * near * far) / (far + near - z * (far - near));

    return vec4(vec3(linear_depth / far), 1.0);
}

void main()
{
    if (visualize_depth)
    {
        frag_color = calculate_depth();
        return;
    }

    vec4 tex = texture(material.diffuse.sampler, text_cord);

    if (tex.a < 0.1)
    {
        discard;
    }

    vec3 result;

    LightingData lighting_data =
    LightingData(
    create_texture(material.diffuse),
    material.specular,
    normalize(normals),
    normalize(view_pos - frag_pos));

    if (recieve_lighting)
    {
        for (int i = 0; i < light_count; i++)
        {
            if (lights[i].is_active)
            {
                result += calculate_lighting(lights[i], lighting_data);
            }
        }
    }
    else
    {
        result += lighting_data.diffuse;
    }

    if (any(greaterThan(material.color, vec3(0.0))))
    {
        if (result.x == 0 && result.y == 0 && result.z == 0)
        {
            result = vec3(1);
        }
        result *= material.color;
    }

    frag_color = vec4(result, tex.a * material.transparency);
}
