#version 430 core

in vec2 texCoord0;
in vec4 color0; 
in flat int outIndex;

//uniform sampler2D tex;
uniform sampler2DArray texArr;

out vec4 fColor;
void main() 
{ 
//    fColor = texture(tex, texCoord0)  * modifier;
    fColor = texture(texArr, vec3(texCoord0, outIndex)) * color0;
}
