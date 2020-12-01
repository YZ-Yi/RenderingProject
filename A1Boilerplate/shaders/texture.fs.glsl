#version 330 core
out vec4 FragColour;

//Input variables, passed in from vertex shader
in vec3 FragPos;
in vec3 Normal;

//Constants, passed in directly from C++
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform vec3 lightPositions;
uniform vec3 lightIntensities;
uniform float rVal;
uniform int shaderNum;
uniform	float z_min;
uniform float opacity;
uniform float hVal;
uniform float sVal;
uniform float vVal;

// texture samplers
uniform sampler2D texture1;

vec3 hsvTorgb(vec3 hsv);
vec3 rgbTohsv(vec3 c);
void main()
{   
    vec2 TexCoords;
  	
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPositions - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 viewDis = viewPos - FragPos;

    float s =  max(dot(norm, lightDir), 0.00);
    float t;

    t = abs(dot(norm, -viewDir));
    t = pow(t, rVal);

    //using hsv color for adjustment

    //convert rgb color to hsv color
    vec3 hsvColor = rgbTohsv(objectColor);

    //if it's blue 
    if(hsvColor.x > 0.5 && hsvColor.x <= 0.75){
         if(s >= 0.5){
            float val = s - 0.5;
            hsvColor.x = hsvColor.x  - val * hVal;
        }
        else{
            float val = 0.5 - s;
            hsvColor.x = val * hVal + hsvColor.x;

        }
    }
    else{
         if(s >= 0.5){
            float val = s - 0.5;
            hsvColor.x = val * hVal + hsvColor.x;
           
        }
        else{
            float val = 0.5 - s;
            hsvColor.x = hsvColor.x  - val * hVal;
        }
    }
 


    //convert hsv color to rgb color
    vec3 objColor = hsvTorgb(hsvColor);

    TexCoords = vec2(s, t); 
        
    vec4 texColour = texture(texture1, TexCoords);
    

    FragColour = vec4(texColour.xyz * objColor * opacity + objColor * (1 - opacity), 1.0);
    //FragColour = vec4(texColour.xyz * objColor * opacity + texColour.xyz * (1 - opacity), 1.0);
    //FragColour = texColour;
    //FragColour = vec4(objColor, 1.0);
    //FragColour = vec4(objectColor, 1.0);
}



//The following block-of-code was adapted from code
//I found at the following URL:
//https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
//convert rgb color to hsv color and convert hsv color to rgb color

vec3 rgbTohsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsvTorgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
//---------------------end of attribute code----------------------------------------------------------------