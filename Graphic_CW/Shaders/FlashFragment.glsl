#version 150
uniform sampler2D diffuseTex;
uniform float flashSpeed;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 FragColor;

void main(void){

	float blendFlash = clamp(flashSpeed, 0.0, 0.5);

	FragColor = texture(diffuseTex, IN.texCoord);
  FragColor.a = blendFlash;
}
