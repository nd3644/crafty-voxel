#version 430 core

in vec2 texCoord0;
in vec4 color0; 
in flat int outIndex;

uniform sampler2D tex;
out vec4 fColor;
void main() 
{ 
    vec4 modifier = vec4(1,1,1,1);
    if(outIndex < 0) {
        modifier = vec4(1,0,0,1);
    }
    else if(outIndex == 1) {
        modifier = vec4(0,0,1,1);
    }
    fColor = texture(tex, texCoord0)  * modifier;
}
