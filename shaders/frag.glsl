#version 330 core

in vec4 Color;
in vec3 Normal;

out vec4 outColor;

//uniform sampler2D tex;
uniform vec3 light_pos;

float lambert(vec3 N, vec3 L)
{
  vec3 nrmN = normalize(N);
  vec3 nrmL = normalize(L);
  float result = dot(nrmN, nrmL);
  return max(result, 0.0);
}

void main() {
  //RGBA of our diffuse color
  vec3 result = Color.rgb * lambert(Normal, light_pos);
  outColor = vec4(result, Color.a);
  //outColor = vec4(Color, 1);
}
