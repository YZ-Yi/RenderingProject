#version 330 core
out vec4 FragColour;

//Input variables, passed in from vertex shader
in vec3 FragPos;
in vec3 Normal;

//Constants, passed in directly from C++
uniform vec3 viewPos;
uniform vec3 objectColour;
uniform vec3 lightPositions[2];
uniform vec3 lightIntensities[2];


void main()
{   
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightIntensities[0];
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPositions[0] - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightIntensities[0];
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightIntensities[0];  
        
    vec3 result = (ambient + diffuse + specular) * objectColour;
    FragColour = vec4(1.0, 1.0, 1.0, 1.0);
}