#version 150

in vec2 texCoord;

out vec4 fColor;

uniform sampler2D tex;

void main() 
{
    fColor = vec4(0.5, 0.5, 0.5, 1) * texture(tex, texCoord);
}
