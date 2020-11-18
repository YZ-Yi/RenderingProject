#version 330 core
out vec4 FragColour;

//Variables, passed in from vertex shader
in vec3 FragPos;
in vec3 Normal;


//Constants, passed in directly from C++
uniform vec3 viewPos;
uniform vec3 objectColour;
uniform float roughness;
uniform vec3 lightPositions[2];
uniform vec3 lightIntensities[2];


void main()
{   
	//Your code goes here!
	//Use functions to be organized! A function for D, G, and F are good starting points!
    FragColour = vec4(objectColour, 1.0);
}
