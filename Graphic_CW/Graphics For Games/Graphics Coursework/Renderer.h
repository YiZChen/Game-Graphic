#pragma  once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/HeightMap.h"
#include "../../nclgl/TextMesh.h"
#include "../../nclgl/ParticleEmitter.h"
#include "DirLight.h"
#include <sstream>
#include <stdio.h>

#define SHADOWSIZE 2048

class  Renderer : public  OGLRenderer {
public:
	Renderer(Window &parent);
	virtual ~Renderer(void);

	virtual  void  RenderScene();
	virtual  void  UpdateScene(float  msec);

	void	SetShaderParticleSize(float f);

	void	CalculateFPS(float f)
	{
		FPS = 1000.0f / f;
	}

protected:
	void          DrawHeightmap();
	void          DrawWater();
	void          DrawSkybox();

	void		  DrawShadoWScene();
	void		  DrawCombinedScene();

	void		  DisplayText();

	void		  DrawText(const std::string &text, const Vector3 &position, const float size = 10.0f, const bool perspective = false);

	void		  DrawLavaParticle();
	void		  DrawRainParticle();
	void		  DrawExplosionParticle();

	void		  ControlElements(float f);

	void		  DrawFlashImage();

	Shader*				  fontShader;
	Shader*				  waterShader;
	Shader*				  shadowShader;
	Shader*				  skyboxShader;
	Shader*				  particleShader;
	Shader*				  heightmapShader;	//directional light shader
	Shader*				  flashImageShader; //screen flash
	
	Mesh*				  quad;

	Font*				  basicFont;
	
	float				  FPS;				//Calculate the frame per second
	float				  lavaFlashWave;	//the lava light's radius changed by holding a key
	float				  lavaDensity;		//wether the volcano should erupt or not is depending on 
											//how many particles are launched per second

	float				  waterRotate;
	float				  eruptionSpeed;	//increase by holding a key to simulate the super eruption

	float				  skyboxRotate;
	float				  skyboxFade;

	float				  skyLightRotation;
	float			      flashSpeed;		//how fast the screen flash would be
	float				  windSpeed;		//how strong the wind would be
	float				  raindensity;		//wether its raining or not
	float				  bombShellDensity;	//the explosion, its same as the eruption particle and raining

	float				  bombTimer;

	Light*				  pointLight;		//Only light the water
	Light*				  lavaLight;		//Increase the radius by holding a key
	DirectionLight*       directionalLight;	//Only light the terrain
	
	GLuint				  cubeMap;			//sky Box day & night
	GLuint				  cubeMap2;
	GLuint				  skyboxDay;
	GLuint			      skyboxNight;

	GLuint				  shadowTex;		//Shadow map
	GLuint				  shadowFBO;

	GLint				  previousMemory;	//calculate memory by (currentMemory - previousMemory)
	GLint				  currentMemory;

	GLuint				  FlashImage;

	Camera*				  camera;			
	HeightMap*			  heightMap;

	ParticleEmitter*      lavaEmitter;		//lava eruption, raining system, explosion
	ParticleEmitter*      rainEmitter;
	ParticleEmitter*	  explosionEmitter;

	Vector4				  lightDir;			//simulate the suns rotation
	
	
	bool				  isWindy;			
	bool				  isRainy;			
	bool				  isDaytime;		
	bool				  isEruption;		
	bool				  isFlashing;		
	bool				  isBombTriggered;
};