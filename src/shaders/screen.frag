#version 460 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D screen_texture;
uniform vec2 resolution;
uniform float gamma;
uniform float exposure;

vec4 pp_pixelate()
{
    float block_size = 250;
    vec2 block_coord = floor(tex_coords * block_size) / block_size;
    vec4 pixelated_color = texture(screen_texture, block_coord);

    return pixelated_color;
}

vec4 pp_compress()
{
    vec4 tex = texture(screen_texture, tex_coords);
    float quality = 15;
    vec3 quantized_color = floor(tex.rgb * quality) / quality;
    return vec4(quantized_color, tex.a);
}

vec4 pp_drugged()
{
    vec4 tex = texture(screen_texture, tex_coords);

    float noise_intensity = 0.6;
    vec3 noisy_color = tex.rgb + noise_intensity * (texture(screen_texture, tex_coords * noise_intensity).rgb - 0.5);

    return vec4(noisy_color, tex.a);
}

vec4 pp_strip()
{
    float stripeDensity = 100.0;
    vec2 stripedCoord = vec2(tex_coords.x, floor(tex_coords.y * stripeDensity) / stripeDensity);
    return texture(screen_texture, stripedCoord);
}

vec4 pp_invert()
{
    return vec4(vec3(1 - texture(screen_texture, tex_coords)), 1);
}

void main()
{
    vec3 screen_color = texture(screen_texture, tex_coords).rgb;
    vec3 mapped = vec3(1.0) - exp(-screen_color * exposure);

    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1);
}