#version 330 core

in vec3 frag_Color;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool enableLighting;

void main(void) 
{
    if (!enableLighting) {
        FragColor = vec4(frag_Color, 1.0);
        return;
    }
    
    // Ambient lighting (매우 약하게)
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // 거리에 따른 감쇠 계산
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    vec3 diffuse = diff * lightColor * attenuation;
    
    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * attenuation;
    
    // 최종 색상 계산
    vec3 result = (ambient + diffuse + specular) * frag_Color;
    FragColor = vec4(result, 1.0);
}