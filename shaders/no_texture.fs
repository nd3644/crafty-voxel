#version 430 core

in vec4 color0; 

out vec4 fColor;
void main() 
{ 
//    fColor = texture(tex, texCoord0)  * modifier;
    fColor = color0;
}
