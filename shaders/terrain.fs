#version 430 core
in vec2 texCoord0;
in vec4 color0; 
uniform sampler2D tex;
out vec4 fColor;
void main() 
{ 
    fColor = texture(tex, texCoord0) * color0; 
}
