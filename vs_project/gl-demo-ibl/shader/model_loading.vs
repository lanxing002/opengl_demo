#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;


out vec2 tex_coords;
out vec3 frag_pos;  //pos in world space
out vec3 normal; //use light in wolrd space


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    tex_coords = aTexCoords;    
    frag_pos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}