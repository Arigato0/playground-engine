#version 460 core

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

uniform float bright_threshold;
uniform float texture_scale;
uniform bool receive_lighting;
uniform bool visualize_depth;
uniform bool flip_normals;
uniform bool contribute_bloom;

uniform float camera_near;
uniform float camera_far;
uniform vec3 view_pos;

uniform float shadow_far;
uniform int pcf_samples;
uniform float shadow_bias;
uniform bool enable_soft_shadows;

in vec3 frag_pos;
in vec3 normals;
in vec2 text_cord;

in mat3 TBN;

struct Texture
{
    sampler2D sampler;
    bool enabled;
};

struct Material
{
    Texture diffuse;
    Texture bump;
    float bump_strength;
    float specular;
    float shininess;
    float emission;
    float transparency;
    vec3 color;
    bool cast_shadow;
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

    samplerCube shadow_map;
};

uniform Material material;

#define MAX_LIGHTS 16
uniform Light lights[MAX_LIGHTS];
uniform int light_count;

uniform samplerCube shadow_maps[2];

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
    float shadow;
    vec3 light_pos;
    vec3 frag_pos;
    vec3 view_pos;
};

LightingData data;

float calculate_penumbra_width(float current_depth, Light light)
{
    float light_distance = length(frag_pos - light.position);
    float penumbra_width = (light_distance - current_depth) / 5;

    return penumbra_width;
}

float calculate_filter_radius(float penumbra_width, float current_depth, float closest_depth)
{
    float filter_radius = penumbra_width * abs(current_depth - closest_depth);

    return filter_radius;
}

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
    float penumbra_width = calculate_penumbra_width(current_depth, light);

    for (int i = 0; i < pcf_samples; ++i)
    {
        float closest_depth = texture(light.shadow_map, frag_to_light + sampling_disk[i] * disk_radius).r;
        closest_depth *= shadow_far;

        float filter_radius = calculate_filter_radius(penumbra_width, current_depth, closest_depth);

        if (current_depth - shadow_bias > closest_depth - filter_radius)
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

    float closest_depth = texture(light.shadow_map, frag_to_light).r;
    closest_depth *= shadow_far;
    float current_depth = length(frag_to_light);
    float bias = 0.05;

    return current_depth - bias > closest_depth ? 1.0 : 0.0;
}

vec3 calculate_lighting(Light light, LightingData data)
{
    data.light_pos = material.bump.enabled ? light.position * TBN : light.position;

    vec3 light_dir = normalize(data.light_pos - data.frag_pos);
    vec3 halfway_dir = normalize(light_dir + data.view_dir);

    float diff = max(dot(data.norm, light_dir), 0.0);

    vec3 diffuse = light.color * light.diffuse * diff * data.diffuse * light.power;
    vec3 ambient =  light.color * data.diffuse * light.ambient * light.power;

    float spec = pow(max(dot(data.norm, halfway_dir), 0.0), material.shininess);
    vec3 specular = light.color * data.specular * spec * light.specular * light.power;

    //return specular + diffuse + ambient;

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

    return ambient + (1 - data.shadow) * (diffuse + specular + material.emission);
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

    if (material.bump.enabled)
    {
        vec3 bump_normal = texture(material.bump.sampler, text_cord).rgb;

        if (flip_normals)
        {
            bump_normal.y = -bump_normal.y;
        }

        bump_normal = bump_normal * 2 - 1.0;
        bump_normal.xy *= material.bump_strength;
        data.norm = normalize(bump_normal);

        data.frag_pos = frag_pos * TBN;
        data.view_pos = view_pos * TBN;
    }
    else
    {
        data.norm = normalize(normals);
        data.frag_pos = frag_pos;
        data.view_pos = view_pos;
    }

    data.diffuse = create_texture(material.diffuse);
    data.specular = material.specular;
    data.view_dir = normalize(data.view_pos - data.frag_pos);
    data.shadow = material.cast_shadow ? 1 : 0;

    if (material.cast_shadow)
    {
        for (int i = 0; i < light_count; i++)
        {
            if (lights[i].is_active)
            {
                data.shadow *= calculate_shadows(lights[i]);
            }
        }
    }

    if (receive_lighting)
    {

        for (int i = 0; i < light_count; i++)
        {
            if (lights[i].is_active)
            {
                result += calculate_lighting(lights[i], data);
            }
        }
    }
    else
    {
        result += data.diffuse;
    }

    if (any(greaterThan(material.color, vec3(0.0))))
    {
        if (result.x == 0 && result.y == 0 && result.z == 0)
        {
            result = vec3(1);
        }
        result *= material.color;
    }

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > bright_threshold && contribute_bloom)
    {
        bright_color = vec4(result, 1);
    }
    else
    {
        bright_color = vec4(0, 0, 0, 1);
    }

    frag_color = vec4(result, tex.a * material.transparency);
}
