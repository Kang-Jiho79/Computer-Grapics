#version 330 core

uniform vec3 in_Color;

out vec4 FragColor;

void main(void) 
{
	FragColor = vec4 (in_Color, 1.0);
}
