#version 150

in vec4 f_colour;
out vec4 out_colour;

int scanHeight = 32;

void main() 
{

    out_colour = f_colour * ((int(gl_FragCoord.y) % scanHeight > scanHeight / 2) ? 0.8 : 1.0);
}
