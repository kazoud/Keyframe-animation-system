#version 410

uniform vec3 uLight, uLight2, uColor;
uniform sampler2D uTexColor;

in vec3 vNormal;
in vec3 vPosition;
in vec2 vTexCoord;

out vec4 fragColor;

void main() {
 
  vec3 tolight = normalize(uLight - vPosition);
  vec3 tolight2 = normalize(uLight2 - vPosition);
  vec3 normal = normalize(vNormal);

  float diffuse = max(0.0, dot(normal, tolight));
  diffuse += max(0.0, dot(normal, tolight2));
  vec3 intensity = uColor * diffuse ;
  vec4 tex_col=texture(uTexColor, vTexCoord);

  fragColor = vec4(intensity, 1.0) * tex_col*(4);
}
