#version 330 core
//Vertexs
layout (location = 0) in vec3 vertexPosition;

// Vertex data out data
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 color;
void main()
{
    color = vertexPosition + 0.5f;    
    gl_Position =  projection * view * model * vec4(vertexPosition, 1.0f);
}
