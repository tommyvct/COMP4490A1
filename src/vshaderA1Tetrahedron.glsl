#version 150

in vec4 vPosition;

uniform mat4 ModelView, Projection;

out vec4 f_colour;

void main()
{
  if (gl_VertexID % 4 == 0) {
    f_colour = vec4(1, 0, 0, 1);
  } else if (gl_VertexID % 4 == 2) {
    f_colour = vec4(0, 1, 0, 1);
  } else if (gl_VertexID % 4 == 3) {
    f_colour = vec4(0, 0, 1, 1);
  } else if (gl_VertexID % 4 == 1) {
    f_colour = vec4(1, 1, 0, 1);
  }

  gl_Position = Projection * ModelView * vPosition;
}

