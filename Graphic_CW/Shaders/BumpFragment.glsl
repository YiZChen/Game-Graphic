#version  150  core

uniform  sampler2D  diffuseTex;
uniform  sampler2D  bumpTex;
uniform  sampler2DShadow  shadowTex;

uniform  vec3    cameraPos;

//directional light
uniform  vec4    DirlightColour;
uniform  vec3    lightDir;

//point flash light
uniform  vec4    lightColour;
uniform  vec3    lightPos;
uniform  float   lightRadius;

uniform  float   lavaFlashWave;

in  Vertex {
    vec3  colour;
    vec2  texCoord;
    vec3  normal;
    vec3  tangent;
    vec3  binormal;
    vec3  worldPos;
    vec4  shadowProj;
} IN;

out  vec4  FragColor;
const vec3 grassland = vec3(0.1,0.2,0.0);
const vec3 lava = vec3(0.8,0.2,0);

void  main(void)    {
  vec4  diffuse = texture(diffuseTex , IN.texCoord );

  //Mixing colours
  float mix1 = clamp((IN.worldPos.y-45.0f)/50f,0.0f,1.0f);
  float mix2 = clamp((IN.worldPos.y-110.0f)/150.0f,0.0f,1.0f);
  vec3 q = mix(grassland,diffuse.rgb,mix1);
  vec3 q1 = mix(q,lava,mix2);

  //recalculate the normals
  mat3  TBN = mat3(IN.tangent , IN.binormal , IN.normal );
  vec3  normal = normalize(TBN * (texture(bumpTex ,IN.texCoord ).rgb * 2.0 - 1.0));

  //directional light incident
  vec3   directionalncident = normalize(lightDir);
  float  directionalLambert = max(0.0, dot(-directionalncident , normal ));

  //pointlight
  vec3   pointIncident = -normalize(IN.worldPos  - cameraPos );
  float  pointLambert = max(0.0, dot(pointIncident , IN.normal ));

  float  dist = length(lightPos  - IN.worldPos );
  float  atten = 1.0 - clamp(dist / lavaFlashWave , 0.3,  1.0);

  vec3  viewDir = normalize(cameraPos  - IN.worldPos );
  vec3  halfDir = normalize(pointIncident + viewDir );
  float  rFactor = max(0.0, dot(halfDir , IN.normal ));
  float  sFactor = pow(rFactor , 50.0 );

  //shadows
  float  shadow = 1;
  if(IN.shadowProj.w > 0.0)
  {
    shadow = textureProj(shadowTex, IN.shadowProj);
  }
  shadow = max(shadow, 0.5f);

  vec3 colour = (q1.rgb * DirlightColour.rgb);

  //if lava light is actived or not
  if(lavaFlashWave > 0.0f)
  {
    colour = (q1.rgb * lightColour.rgb);
    colour += (lightColour.rgb * sFactor) * 0.33;
    FragColor = vec4(lightColour.rgb * colour * pointLambert * atten, diffuse.a) + vec4(q1.rgb * DirlightColour.rgb * directionalLambert * shadow , diffuse.a);
    //FragColor.rgb += (diffuse.rgb * ) * 0.1;
    //FragColor.rgb = lightColour.rgb;
  }
  else
  {
      FragColor = vec4(colour * directionalLambert * shadow , diffuse.a);
  }
  //FragColor.rgb = vec3(atten);
  //FragColor.rgb = vec3(shadow);
}
