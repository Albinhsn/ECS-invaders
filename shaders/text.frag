#version 450 core



in vec2 UV;

uniform sampler2D texture1;
uniform vec4 color;
out vec4 FragColor;

float RGBMedian(vec3 Sample)
{
  return max(min(Sample.r, Sample.g), min(max(Sample.r, Sample.g), Sample.b));
}

float ColorDistance(float Color)
{
  return Color - 0.5f;
}
void main()
{

  vec4 Sample = texture(texture1, UV);

  float Color = RGBMedian(Sample.rgb);
  float Distance = ColorDistance(Color);
  if(Distance < 0)
  {
    discard;
  }

  FragColor = color;
}