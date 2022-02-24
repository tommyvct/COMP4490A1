#version 150

in vec4 vPosition;

uniform mat4 ModelView, Projection;

flat out vec4 f_colour;

void main()
{
  // cube: give different faces different colours
  if ((gl_VertexID + 1) % 3 == 0) {
    f_colour = vec4(1,0,0,1);
  } else if ((gl_VertexID + 1) % 3 == 1) {
    f_colour = vec4(0,1,0,1);
  } else if ((gl_VertexID + 1) % 3 == 2) {
    f_colour = vec4(0,0,1,1);
  } else{
    f_colour = vec4(0.5, 0.5, 0.5, 1);
  }

  gl_Position = Projection * ModelView * vPosition;
}

