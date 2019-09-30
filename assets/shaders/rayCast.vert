#version 330 core
layout (location = 0) in vec3 vertexPosition;

out vec3 entryPoint;

uniform mat4 MVP;

void main()
{
    entryPoint = vertexPosition + 0.5f;
    gl_Position = MVP * vec4(vertexPosition,1.0);
}