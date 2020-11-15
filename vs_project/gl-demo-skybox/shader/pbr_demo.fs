#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec3 TexCoords;

uniform mat4 rotate;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{             
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
    vec3 TexCoords2 = vec3(rotate * vec4(TexCoords, 1.0));
    FragColor = texture(skybox, TexCoords);
    //FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}