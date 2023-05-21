#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;
layout(location = 3) in int index;

out vec2 texCoord0;
out vec4 color0;
out flat int outIndex;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Proj;

void main() 
{ 
    texCoord0 = vec2(vTexCoord.x, vTexCoord.y); 
    color0 = vColor;
    gl_Position = Proj * View * Model * vec4(vPosition.x / 800, vPosition.y / 600, 0, 1);
    outIndex = index;
}