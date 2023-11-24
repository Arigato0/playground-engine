#version 460 core

out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;

void main()
{
    FragColor = vec4(1);

    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 result = ambient * object_color;

    FragColor *= vec4(result, 1.0);
}
