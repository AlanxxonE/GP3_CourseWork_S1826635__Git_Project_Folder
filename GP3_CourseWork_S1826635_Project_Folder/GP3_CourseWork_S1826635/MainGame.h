#pragma once
#include <SDL\SDL.h>
#include <SDL/SDL_mixer.h>
#include <GL/glew.h>
#include "Display.h" 
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "transform.h"
#include "SkyBox.h"
#include "GameObject.h"
#include "SDLAudio.h"
#include "DeltaTime.h"
#include "glm/ext.hpp"
#include "RayCaster.h"

enum class GameState{PLAY, EXIT};

class MainGame
{
public:
	MainGame();
	~MainGame();

	void run();

private:

	void initSystems();
	void processInput();
	void gameLoop();
	void drawGame();
	void linkFogShader();
	void linkEmapping();

	void drawSkyBox();
	void drawAsteriods();
	void drawShip();
	void drawMissiles();

	void fireMissiles(int missileNumber);

	void initModels(GameObject*& asteroid);
	bool collision(glm::vec3 *m1Pos, float m1Rad, glm::vec3 *m2Pos, float m2Rad);

	void UpdateDeltaSpeed();

	void createScreenQuad();

	void generateFBO(float w, float h);
	void bindFBO();
	void unbindFBO();
	void renderFBO();

	Display _gameDisplay;
	GameState _gameState;
	Mesh rockMesh;
	Mesh shipMesh;
	Mesh missileMesh;
	Camera myCamera;
	Shader nShader;
	Shader fogShader;
	Shader shaderSkybox;
	Shader eMapping;
	Shader FBOShader;
	SDLAudio gameAudio;

	DeltaTime deltaTime;
	double kSpeed;
	double deltaSpeed;

	RayCaster rayCaster;

	const Uint8* keyboard_state_array;
	bool engaging;

	int missileCounter;

	GameObject* asteroid = new GameObject[20];
	float asDir[20] = { 0 };
	GameObject* missiles = new GameObject[20];	
	GameObject ship;

	Texture texture;

	GLuint FBO;
	GLuint RBO;
	GLuint CBO;

	GLuint quadVAO;
	GLuint quadVBO;

	Skybox skybox;

	vector<std::string> faces;
	
	bool shake = false;
	float counter;
};

