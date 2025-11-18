#version 330 core

layout (location = 0) in vec3 in_Position; 
layout (location = 1) in vec3 in_Color;
layout (location = 2) in vec3 in_Normal;

out vec3 frag_Color;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 Matrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool enableLighting;

void main(void) 
{
    vec4 worldPos = model * vec4(in_Position, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * in_Normal;
    
    gl_Position = Matrix * vec4(in_Position, 1.0);
    frag_Color = in_Color;
}