#version 460 core


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

    float noise_intensity = 0.4;
    vec3 noisy_color = tex.rgb + noise_intensity * (texture(screen_texture, tex_coords * noise_intensity).rgb - 0.5);

    return vec4(noisy_color, tex.a);
}

vec4 pp_strip()
{
    float stripeDensity = 100.0;
    vec2 stripedCoord = vec2(tex_coords.x, floor(tex_coords.y * stripeDensity) / stripeDensity);
    return texture(screen_texture, stripedCoord);
}

vec4 crt()
{
    vec3 original_color = texture(screen_texture, tex_coords).rgb;

    // Simulate scanlines
    float scanline_intensity = 1.5;
    vec3 scanline_color = mix(original_color, vec3(0.0), scanline_intensity);

    // Simulate color bleeding
    float bleeding_amount = 1.105;
    vec3 bleed_color = original_color - bleeding_amount * (original_color - vec3(1.0));

    // Combine scanlines and color bleeding
    vec3 crt_color = mix(bleed_color, scanline_color, 0.5);

    // Simulate barrel distortion
    vec2 distorted_coord = tex_coords - 0.5;
    float distortion = 0.5;
    distorted_coord *= 1.0 + distortion * dot(distorted_coord, distorted_coord);

     if (length(distorted_coord) > 0.5)
     {
        crt_color = vec3(0.0);
    }

    // Sample the final color from the distorted coordinates
    return texture(screen_texture, distorted_coord + 1.5);
}

