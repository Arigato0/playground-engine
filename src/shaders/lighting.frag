#version 460 core

out vec4 frag_color;

uniform vec3 view_pos;
uniform float texture_scale;
uniform bool recieve_lighting;
uniform bool visualize_depth;

uniform float camera_near;
uniform float camera_far;

uniform float shadow_far;
uniform samplerCube shadow_map;
uniform int pcf_samples;
uniform float shadow_bias;
uniform bool enable_soft_shadows;

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

vec3 sampling_disk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float calculate_pcf(Light light)
{
    vec3 frag_to_light = frag_pos - light.position;

    float current_depth = length(frag_to_light);

    float shadow = 0.0;
    float view_distance = length(view_pos - frag_pos);
    float disk_radius = (1.0 + (view_distance / shadow_far)) / shadow_far;

    for (int i = 0; i < pcf_samples; ++i)
    {
        float closest_depth = texture(shadow_map, frag_to_light + sampling_disk[i] * disk_radius).r;

        closest_depth *= shadow_far;

        if (current_depth - shadow_bias > closest_depth)
        {
            shadow += 1.0;
        }
    }

    shadow /= float(pcf_samples);

    return shadow;
}

float calculate_shadows(Light light)
{
    if (enable_soft_shadows)
    {
        return calculate_pcf(light);
    }

    vec3 frag_to_light = frag_pos - light.position;
    float closest_depth = texture(shadow_map, frag_to_light).r;
    closest_depth *= shadow_far;
    float current_depth = length(frag_to_light);
    float bias = 0.05;

    return current_depth - bias > closest_depth ? 1.0 : 0.0;
}

vec3 calculate_lighting(Light light, LightingData data)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 halfway_dir = normalize(light_dir + data.view_dir);

    float diff = max(dot(data.norm, light_dir), 0.0);

    vec3 diffuse = light.color * light.diffuse * diff * data.diffuse * light.power;
    vec3 ambient =  light.color * data.diffuse * light.ambient * light.power;

    float spec = pow(max(dot(data.norm, halfway_dir), 0.0), material.shininess);
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

    float shadow = calculate_shadows(light);

    return ambient + (1 - shadow) * (diffuse + specular);
}

vec4 calculate_depth()
{
    float z = gl_FragCoord.z * 2.0 - 1.0;
    float linear_depth = (2.0 * camera_near * camera_far) / (camera_far + camera_near - z * (camera_far - camera_near));

    return vec4(vec3(linear_depth / camera_far), 1.0);
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
