#version 450 core



in vec2 UV;

uniform sampler2D texture1;
out vec4 FragColor;

void main()
{

  vec4 Sample = texture(texture1, UV);
  if(Sample.a == 0)
  {
    discard;
  }
  FragColor = Sample;
}