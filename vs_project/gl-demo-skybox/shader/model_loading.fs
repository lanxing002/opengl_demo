#version 330 core
out vec4 FragColor;

in vec2 tex_coords;
in vec3 frag_pos;
in vec3 normal;

uniform vec3 camera_pos;
uniform sampler2D texture_diffuse1;
uniform samplerCube texture_env;

void main()
{    
    vec3 I = normalize(frag_pos - camera_pos);
    //normal = normalize(normalize);
    vec3 fle = reflect(I, normalize(normal));
    
    float ratio = 1.00 / 1.52;
    vec3 fra = refract(I, normalize(normal), ratio);

    FragColor = 0.5 *( texture(texture_env, fle) + texture(texture_env, fra));

    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0); //  ”≤ÓÕº
}