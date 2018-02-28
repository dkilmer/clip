#version 330 core

in vec3 position;
in vec4 color;
in vec3 normal;

out vec4 Color;
out vec3 Normal;

uniform mat4 vp;

void main()
{
	Color = color;
	Normal = normal;
  gl_Position = vp * vec4(position, 1.0);
}
