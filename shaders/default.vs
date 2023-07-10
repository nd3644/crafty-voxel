#version 430 core

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;
layout(location = 3) in int index;

out vec2 texCoord0;
out vec4 color0;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Proj;

void main() 
{ 
    texCoord0 = vec2(vTexCoord.x, vTexCoord.y); 
    color0 = vColor;
    gl_Position = Proj * vec4(vPosition.x, vPosition.y, 0, 1);
}