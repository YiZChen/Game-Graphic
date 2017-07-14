#include "Renderer.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent) {
	//Get previous memory
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &previousMemory);
	
	//Camera
	camera = new  Camera();
	camera->SetPosition(Vector3(3600, 400.0f, 4000));
	camera->SetYaw(45.0f);

	//Shaders
	waterShader = new Shader(SHADERDIR"PerPixelVertex.glsl", SHADERDIR"reflectFragment.glsl");
	skyboxShader = new Shader(SHADERDIR"skyboxVertex.glsl", SHADERDIR"skyboxFragment.glsl");
	fontShader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	shadowShader = new Shader(SHADERDIR"ShadowVertex.glsl", SHADERDIR"ShadowFragment.glsl");
	particleShader = new Shader(SHADERDIR"ParticleVertex.glsl", SHADERDIR"ParticleFragment.glsl", SHADERDIR"ParticleGeometry.glsl");
	heightmapShader = new Shader(SHADERDIR"BumpVertex.glsl", SHADERDIR"BumpFragment.glsl");
	flashImageShader = new Shader(SHADERDIR"FlashVertex.glsl", SHADERDIR"FlashFragment.glsl");

	//Set hightmap and textures
	heightMap = new HeightMap(TEXTUREDIR"island.raw");
	heightMap->SetTexture1(SOIL_load_OGL_texture(TEXTUREDIR"terrain_Diffuse.tga",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	heightMap->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"terrain_Normal.tga",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	//Set water
	quad = Mesh::GenerateQuad();
	quad->SetTexture1(SOIL_load_OGL_texture(TEXTUREDIR"water.jpg",
		SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	waterRotate = 0.1f;
	
	//SetLights
	lightDir.x = 1.0f;
	lightDir.y = -1.0f;
	lightDir.z = 0.0f;
	lightDir.w = 1.0f;
	// change its direction every second
	directionalLight = new  DirectionLight(Vector3(lightDir.x, lightDir.y, lightDir.z),Vector4(1.0f, 1.0f, 1.0f, 1));
	skyLightRotation = 1.0f;
	
	//Point Light only cast water
	pointLight = new Light(Vector3((RAW_HEIGHT*HEIGHTMAP_X / 2.5f),
					 800.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.5f)),
					 Vector4(1.0, 1.0, 1.0, 0.3f), 
					 (RAW_WIDTH*HEIGHTMAP_X) / 1.0f);

	//Volcano Light which can change the radius by holding a key
	isEruption = true;
	lavaLight = new Light(Vector3((RAW_HEIGHT*HEIGHTMAP_X / 2.5f),
		500.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.5f)),
		Vector4(1.0, 0.0, 0.0, 1.0f), lavaFlashWave);
	lavaFlashWave = (RAW_WIDTH*HEIGHTMAP_X) / 5.0f;

	//Day and night skybox 
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"Sky_Day_LF.tga", TEXTUREDIR"Sky_Day_RT.tga",
		TEXTUREDIR"Sky_Day_UP.tga", TEXTUREDIR"Sky_Day_DN.tga",
		TEXTUREDIR"Sky_Day_FT.tga", TEXTUREDIR"Sky_Day_BK.tga",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	cubeMap2 = SOIL_load_OGL_cubemap(
		TEXTUREDIR"Sky_Night_LF.tga", TEXTUREDIR"Sky_Night_RT.tga",
		TEXTUREDIR"Sky_Night_UP.tga", TEXTUREDIR"Sky_Night_DN.tga",
		TEXTUREDIR"Sky_Night_FT.tga", TEXTUREDIR"Sky_Night_BK.tga",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	skyboxRotate = 1.0f;
	skyboxFade = 0.0f;
	isDaytime = false;

	//Set font
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"Font.png", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);

	//set particle system
	//the first boolean parameter is to define wether the emitter is area spawner or not	
	//the last boolean parameter is to define wether the emitter can spawn to all direction or not
	lavaEmitter = new ParticleEmitter(false, TEXTUREDIR"particle.tga" ,false);
	rainEmitter = new ParticleEmitter(true, TEXTUREDIR"ParticleRain.png" ,false);
	explosionEmitter = new ParticleEmitter(false, TEXTUREDIR"particle.tga" ,true);
	isWindy = false;
	isRainy = false;
	isBombTriggered = false;
	windSpeed = 0;
	eruptionSpeed = 0.0f;
	raindensity = 0;
	lavaDensity = 0;
	bombShellDensity = 0;
	bombTimer = 0.0;

	//screen flash
	FlashImage = SOIL_load_OGL_texture(TEXTUREDIR"Flash.png",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	flashSpeed = 0;

	if (!waterShader->LinkProgram() || !heightmapShader->LinkProgram() || 
		!skyboxShader->LinkProgram() || !fontShader->LinkProgram() || 
		!shadowShader->LinkProgram() || !particleShader->LinkProgram() || !flashImageShader->LinkProgram())
	{
		return;
	}
	if (!cubeMap || !cubeMap2 || !quad->GetTexture() || 
		!heightMap->GetTexture() || !heightMap->GetBumpMap()
		  || !basicFont || !FlashImage)
	{
		return;
	}

	//Create Shadow texture and bind it to frame Buffer
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	        SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	SetTextureRepeating(quad->GetTexture(), true);
	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	init = true;
}

Renderer::~Renderer(void) {
	delete  camera;
	delete  heightMap;
	delete  quad;
	delete  waterShader;
	delete  skyboxShader;
	delete  heightmapShader;
	delete  flashImageShader;
	delete  directionalLight;
	delete	lavaLight;
	delete  particleShader;
	delete  pointLight;
	delete  basicFont;
	delete  shadowShader;
	delete  lavaEmitter;
	delete  rainEmitter;
	delete  explosionEmitter;
}

void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();

	waterRotate += msec / 10000.0f;
	ControlElements(msec);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	DrawSkybox();

	DrawShadoWScene();

	DrawCombinedScene();

	DrawWater();

	DrawLavaParticle();

	DrawRainParticle();

	DrawExplosionParticle();

	DisplayText();

	DrawFlashImage();

	SwapBuffers();

	//Get current memory
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemory);
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	SetCurrentShader(skyboxShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "cubeTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "cubeTex2"), 3);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "skyFade"), skyboxFade);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap2);

	viewMatrix = viewMatrix * Matrix4::Rotation(skyboxRotate, Vector3(0, 1, 0));

	UpdateShaderMatrices();
	quad->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {

	modelMatrix.ToIdentity();

	textureMatrix.ToIdentity();

	Matrix4 tempMatrix = shadowMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, *&tempMatrix.values);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, *&modelMatrix.values);

	heightMap->Draw();

	glUseProgram(0);
}

void Renderer::DrawWater() {
	SetCurrentShader(waterShader);
	//point Light->shader
	SetShaderLight(*pointLight);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float *)& camera->GetPosition());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	float  heightX = (RAW_WIDTH*HEIGHTMAP_X / 2.0f);

	float  heightY = 256 * HEIGHTMAP_Y / 7.0f;

	float  heightZ = (RAW_HEIGHT*HEIGHTMAP_Z / 2.0f);

	modelMatrix =
		Matrix4::Translation(Vector3(heightX, heightY, heightZ)) *
		Matrix4::Scale(Vector3(10000, 1, 10000)) *
		Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));

	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(waterRotate, Vector3(0.0f, 0.0f, 1.0f));

	UpdateShaderMatrices();

	quad->Draw();

	glUseProgram(0);
}

void Renderer::DrawShadoWScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	//glViewport(1, 1, SHADOWSIZE -2, SHADOWSIZE - 2);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	SetCurrentShader(shadowShader);

	//draw the shadow map along the direction of directional light
	Vector3 terrain_centre = Vector3(RAW_WIDTH*HEIGHTMAP_X * 0.5f, 0, RAW_WIDTH*HEIGHTMAP_Z * 0.5f);
	viewMatrix = Matrix4::BuildViewMatrix(terrain_centre, terrain_centre + Vector3(lightDir.x, lightDir.y, lightDir.z));

	projMatrix = Matrix4::Orthographic(-5000.0f, 5000.0f, RAW_WIDTH*HEIGHTMAP_X * 0.5f, -RAW_WIDTH*HEIGHTMAP_X * 0.5f, RAW_WIDTH*HEIGHTMAP_X * 0.5f, -RAW_WIDTH*HEIGHTMAP_X * 0.5f);

	shadowMatrix = biasMatrix * (projMatrix * viewMatrix);
	
	UpdateShaderMatrices();
	
	DrawHeightmap();
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawCombinedScene()
{
	//shader with shadow
	SetCurrentShader(heightmapShader);

	//directional Light->shader
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightDir"), 1, (float*)&directionalLight->GetDirection());
	glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "DirlightColour"), 1, (float*)&directionalLight->GetColour());
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float *)& camera->GetPosition());

	//Set lava flash point light, I use lavaFLashWave instead of LightRadius
	SetShaderLight(*lavaLight);

	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "lavaFlashWave"), lavaFlashWave);

	//texture->shader
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 6);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	UpdateShaderMatrices();

	DrawHeightmap();

	glUseProgram(0);
}

void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective) {
	TextMesh* mesh = new TextMesh(text, *basicFont);

	if (perspective) {
		textureMatrix.ToIdentity();
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	else {//orthographic
		textureMatrix.ToIdentity();
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	}

	SetCurrentShader(fontShader);
	glUseProgram(fontShader->GetProgram());
	glUniform1i(glGetUniformLocation(fontShader->GetProgram(), "diffuseTex"), 0);

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);
	mesh->Draw();
	glDepthMask(GL_TRUE);

	glUseProgram(0);

	delete mesh; 
}

void Renderer::DisplayText()
{
	ostringstream fps;
	fps << "FPS:" << FPS;

	ostringstream memory;
	memory << "Memory Usage:" << ((float)(previousMemory - currentMemory))/1024;
	
	DrawText(fps.str(), Vector3(0, 0, 0), 20.0f, false);
	DrawText(memory.str(), Vector3(0, 20, 0), 20.0f, false);

	DrawText("VOLCANO ISLAND", Vector3((RAW_HEIGHT*HEIGHTMAP_X / 3.5f), 1100.f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.0f)), 100.0f, true);
	
	DrawText("Press 1: Explosion", Vector3(0, 700, 0), 20.0f, false);
	DrawText("Press 2: Eruption", Vector3(0, 720, 0), 20.0f, false);
	DrawText("Press 3: Raining", Vector3(0, 740, 0), 20.0f, false);
	DrawText("Press 4: Rest", Vector3(0, 760, 0), 20.0f, false);

	DrawText("Hold E: Increase Eruption", Vector3(0, 780, 0), 20.0f, false);
	DrawText("Hold Q: Cause WindEffect", Vector3(0, 800, 0), 20.0f, false);

	DrawText("YizChen's Coursework", Vector3(1000, 800, 0), 20.0f, false);
}

void Renderer::DrawLavaParticle()
{
	glClearColor(0, 0, 0, 1);
	SetCurrentShader(particleShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	SetShaderParticleSize(lavaEmitter->GetParticleSize());

	modelMatrix = Matrix4::Translation(Vector3((RAW_HEIGHT*HEIGHTMAP_X / 2.3f), 200.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.1f)));
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	
	lavaEmitter->SetParticleSize(80.0f);
	lavaEmitter->SetParticleVariance(1.0f);
	lavaEmitter->SetLaunchParticles(lavaDensity);
	lavaEmitter->SetParticleLifetime(6000.0f);
	lavaEmitter->SetParticleSpeed(eruptionSpeed);

	UpdateShaderMatrices();

	glDepthMask(GL_FALSE);

	lavaEmitter->Draw();

	glDepthMask(GL_TRUE);
	
	glUseProgram(0);
}

void Renderer::DrawRainParticle()
{
	glClearColor(0, 0, 0, 1);
	SetCurrentShader(particleShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	SetShaderParticleSize(rainEmitter->GetParticleSize());

	modelMatrix = Matrix4::Translation(Vector3((RAW_HEIGHT*HEIGHTMAP_X / 20), 0.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 10)));
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	//Draw Rain
	rainEmitter->SetParticleSize(25.0f);
	//rainEmitter->SetParticleVariance(1.0f);
	rainEmitter->SetLaunchParticles(raindensity);
	rainEmitter->SetParticleLifetime(8000.0f);
	rainEmitter->SetParticleSpeed(-2.0f);

	UpdateShaderMatrices();

	glDepthMask(GL_FALSE);

	rainEmitter->Draw();
	
	glDepthMask(GL_TRUE);

	glUseProgram(0);
}

void Renderer::DrawExplosionParticle()
{
	glClearColor(0, 0, 0, 1);
	SetCurrentShader(particleShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	SetShaderParticleSize(explosionEmitter->GetParticleSize());

	modelMatrix = Matrix4::Translation(Vector3((RAW_HEIGHT*HEIGHTMAP_X / 2.3f), 250.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.1f)));
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	//Draw Explosion
	explosionEmitter->SetParticleSize(50.0f);
	explosionEmitter->SetParticleVariance(1.0f);
	explosionEmitter->SetLaunchParticles(bombShellDensity);
	explosionEmitter->SetParticleLifetime(2000.0f);
	explosionEmitter->SetParticleSpeed(2.0f);

	UpdateShaderMatrices();

	glDepthMask(GL_FALSE);

	explosionEmitter->Draw();

	glDepthMask(GL_TRUE);

	glUseProgram(0);
}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

void Renderer::ControlElements(float msec)
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		isBombTriggered = true;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
	{
		isEruption = true;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
	{
		isRainy = true;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4))
	{
		isEruption = false;
		isRainy = false;
		isBombTriggered = false;
	}

	//Control Wind power
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q))
	{
		isWindy = true;
	}
	else
	{
		isWindy = false;
	}
	if (!isWindy)
	{
		windSpeed -= msec * 0.005f;
		{
			if (windSpeed <= 0)
			{
				windSpeed = 0.0f;
			}
		}
	}
	else
	{
		windSpeed += msec * 0.02f;
		{
			if (windSpeed >= 10)
			{
				windSpeed = 10.0f;
			}
		}
	}

	//Control lava eruption power 
	if (isEruption)
	{
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_E))
		{
			lavaFlashWave += 2 * msec;
			eruptionSpeed += msec * 0.02f;
			if (eruptionSpeed >= 2.0f)
			{
				eruptionSpeed = 2.0f;
			}
			if (lavaFlashWave >= (RAW_WIDTH*HEIGHTMAP_X) / 2.0f)
			{
				lavaFlashWave = (RAW_WIDTH*HEIGHTMAP_X) / 2.0f;
			}
		}
		else
		{
			lavaFlashWave -= 2 * msec;
			eruptionSpeed -= msec * 0.01f;
			if (eruptionSpeed <= 0.5)
			{
				eruptionSpeed = 0.5;
			}
			if (lavaFlashWave <= (RAW_WIDTH*HEIGHTMAP_X) / 5.0f)
			{
				lavaFlashWave = (RAW_WIDTH*HEIGHTMAP_X) / 5.0f;
			}
		}
	}
	
	//Sun rotate
	lightDir = Matrix4::Rotation(0 + (msec * 360) / 50000.0f, Vector3(1, 1, 0)) * lightDir;

	//Dynamic skybox
	skyboxRotate += (msec / 1000000.0f) * 360.f;
	if (!isDaytime)
	{
		skyboxFade += msec / 16000.0f;

		if (skyboxFade >= 1.5)
		{
			skyboxFade = 1;
			isDaytime = true;
		}
	}
	else
	{
		skyboxFade -= msec / 16000.0f;

		if (skyboxFade <= -0.5)
		{
			skyboxFade = 0;
			isDaytime = false;
		}
	}

	//Control rain
	if (!isRainy)
	{
		raindensity -= msec * 0.05f;
		if (raindensity <= 0)
		{
			raindensity = 0;
		}
	}
	else
	{
		raindensity += msec * 0.1f;
		if (raindensity >= 100)
		{
			raindensity = 100;
		}
	}

	//control lava lightRadius and lava density
	if (isEruption)
	{
		lavaDensity += msec * 0.2f;

		if (lavaDensity >= 30)
		{
			lavaDensity = 30;
		}
	}
	else
	{
		lavaDensity -= msec * 0.2f;
	
		if (lavaDensity <= 0)
		{
			lavaDensity = 0;
		}
	}

	//Bomb explosion
	if (isBombTriggered)
	{
		bombShellDensity += msec * 20;
		bombTimer += msec * 0.001f;
		isFlashing = true;
		if (bombShellDensity >= 2000)
		{
			isBombTriggered = false;
			isFlashing = false;
		}
	}
	else
	{
		isFlashing = false;
		bombTimer = 0.0f;
		bombShellDensity = 0;
	}

	//timer to keep track the lifetime of the explosion and flash
	if (bombTimer >= 0.4f)
	{
		isBombTriggered = false;
		bombTimer = 0.0f;
	}

	//Screen Flash
	if (isFlashing)
	{
		flashSpeed = 0.3;
		if (flashSpeed >= 1)
		{
			isFlashing = false;
		}
	}
	else
	{
		flashSpeed -= msec * 0.001 / 2;
		if (flashSpeed < 0)
		{
			flashSpeed = 0;
		}
	}

	rainEmitter->SetWindPower(windSpeed);
	lavaEmitter->SetWindPower(windSpeed);
	explosionEmitter->SetWindPower(windSpeed);

	lavaEmitter->Update(msec);
	rainEmitter->Update(msec);
	explosionEmitter->Update(msec);
}

void Renderer::DrawFlashImage()
{
	SetCurrentShader(flashImageShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 5);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "flashSpeed"), flashSpeed);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, FlashImage);

	UpdateShaderMatrices();

	quad->Draw();

	glUseProgram(0);

	//in case something just go crazy because of the matrix
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();
}