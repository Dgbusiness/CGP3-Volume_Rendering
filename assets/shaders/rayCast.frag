#version 330 core
in vec3 entryPoint;

out vec4 FragColor;

uniform vec2 screenSize;
uniform sampler3D volTexture;
uniform sampler2D bfTexture;
uniform float Step = 1.f/256.f;

void main()
{
    //We took the real exit point on texture
    vec3 exitPoint = texture(bfTexture, gl_FragCoord.st/screenSize).rgb; 
    vec3 vecDir =  exitPoint - entryPoint;       //Calculate the direction of the ray
    float rayLen = length(vecDir);              //Calculate the lenght of the dir vector
    vec3 ray = normalize(vecDir);               //Normalize the dir vector
    vec3 rayStep = ray * Step;

    //We start the ray casting
    vec4 color = vec4(vec3(0.f), 1.f);
    vec3 tr = entryPoint;
    
    for(float i = 0; i < rayLen; i += Step) {
        color.rgb += texture(volTexture,tr).r * vec3(texture(volTexture,tr).r) * color.a;
		color.a *= 1 - texture(volTexture,tr).r;
		if(1-color.a >= 0.99f) break;
        tr += rayStep;
    }
    color.a = 1.0f;
    FragColor = color;
}