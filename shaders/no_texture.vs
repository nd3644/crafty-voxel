#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;

out vec4 color0;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Proj;

void main() 
{ 
    color0 = vColor;
    gl_Position = Proj * View * Model * vec4(vPosition.x, vPosition.y, vPosition.z, 1);
}