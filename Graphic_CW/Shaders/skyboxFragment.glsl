#version  150  core

uniform  samplerCube   cubeTex;
uniform  samplerCube   cubeTex2;
uniform  vec3          cameraPos;
uniform  float         skyFade;

in  Vertex {
    vec3  normal;
} IN;

out  vec4  FragColor;

void  main(void)    {
    vec4  sky1 = texture(cubeTex ,normalize(IN.normal ));
    vec4  sky2 = texture(cubeTex2 ,normalize(IN.normal ));

    float blendColor = clamp(skyFade, 0.0f, 1.0f);

    FragColor = mix(sky1 , sky2 , blendColor);
}
