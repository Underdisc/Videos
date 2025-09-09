#type fragment

in vec2 iUv;
out vec4 iFinalColor;

uniform sampler2D uTexture;

void main() {
  vec4 color = texture(uTexture, iUv);
  vec3 intenseOverflow = max(color.rgb - vec3(1.0, 1.0, 1.0), 0.0);
  iFinalColor = vec4(intenseOverflow, 1.0);
}
