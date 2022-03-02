#version 150 

in vec4 vPosition;
in vec2 vTexCoord;

uniform mat4 ModelView, Projection;

out vec2 texCoord;



void main()
{
    texCoord = vTexCoord;
    gl_Position = Projection * ModelView * vPosition;
}
