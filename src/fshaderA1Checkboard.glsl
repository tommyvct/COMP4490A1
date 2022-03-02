#version 150

in vec2 texCoord;

out vec4 fColor;

uniform sampler2D tex;

int scanHeight = 32;


void main() 
{
    fColor = vec4(0.5, 0.5, 0.5, 1) * texture(tex, texCoord) * ((int(gl_FragCoord.y) % scanHeight > scanHeight / 2) ? 0.8 : 1.2);
}
