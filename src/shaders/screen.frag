#version 460 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D bloom_texture;
uniform sampler2D screen_texture;
uniform vec2 resolution;
uniform float gamma;
uniform float exposure;
uniform bool enable_bloom;

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

// tonemapping functions yoinked from https://github.com/dmnsgn/glsl-tone-map and https://64.github.io/tonemapping/

vec3 filmic(vec3 x)
{
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}

vec3 hable_tonemap_partial(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 hable_filmic(vec3 v)
{
    vec3 curr = hable_tonemap_partial(v * exposure);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / hable_tonemap_partial(W);
    return curr * white_scale;
}

vec3 reinhard(vec3 color)
{
    return vec3(1.0) - exp(-color * exposure);
}

vec3 reinhard2(vec3 x)
{
  const float L_white = 4.0;

  return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

vec3 lottes(vec3 x)
{
  const vec3 a = vec3(1.6);
  const vec3 d = vec3(0.977);
  const vec3 hdrMax = vec3(10.0);
  const vec3 midIn = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b =
      (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
      (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b)
{
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

vec3 uchimura(vec3 x)
{
  const float P = 1.0;  // max display brightness
  const float a = 1.0;  // contrast
  const float m = 0.22; // linear section start
  const float l = 0.4;  // linear section length
  const float c = 1.33; // black
  const float b = 0.0;  // pedestal

  return uchimura(x, P, a, m, l, c, b);
}

void main()
{
    vec3 screen_color = texture(screen_texture, tex_coords).rgb;

    if (enable_bloom)
    {
        screen_color += texture(bloom_texture, tex_coords).rgb;
    }

    vec3 mapped = hable_filmic(screen_color);

    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1);
}
