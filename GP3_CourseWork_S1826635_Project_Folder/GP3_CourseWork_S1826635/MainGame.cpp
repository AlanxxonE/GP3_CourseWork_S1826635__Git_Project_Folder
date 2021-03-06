#include "MainGame.h"
#include "Camera.h"
#include <iostream>
#include <string>

MainGame::MainGame()
{
	_gameState = GameState::PLAY;
	Display* _gameDisplay = new Display(); //new display
}

MainGame::~MainGame()
{
}

void MainGame::run()
{
	initSystems(); 
	gameLoop();
}

void MainGame::initSystems()
{
	_gameDisplay.initDisplay();
	texture.load("..\\res\\AstroShipTexture.jpg");
	rockMesh.loadModel("..\\res\\Asteroid.obj");
	shipMesh.loadModel("..\\res\\NewShip.obj");
	missileMesh.loadModel("..\\res\\NewMissile.obj");
	nShader.init("..\\res\\shaderVert.vert", "..\\res\\shaderFrag.frag");
	fogShader.init("..\\res\\fogShader.vert", "..\\res\\fogShader.frag");
	eMapping.init("..\\res\\shaderReflection.vert", "..\\res\\shaderReflection.frag");
	FBOShader.init("..\\res\\FBOShader.vert", "..\\res\\FBOShader.frag");
	
	gameAudio.addAudioTrack("..\\res\\background.wav");
	gameAudio.addSoundEffect("..\\res\\bang.wav");
	gameAudio.addSoundEffect("..\\res\\homing.wav");
	gameAudio.addSoundEffect("..\\res\\beep.wav");

	myCamera.initCamera(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y + 20, -50), 70.0f, (float)_gameDisplay.getWidth()/_gameDisplay.getHeight(), 1.0f, 1000.0f);
	myCamera.setLook(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z));
	myCamera.setPos(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y + 20, ship.getTM().GetPos()->z - 50));
	
	initModels(asteroid);

	generateFBO(_gameDisplay.getWidth(), _gameDisplay.getHeight());

	createScreenQuad();

	counter = 1.0f;

	vector<std::string> faces
	{
		"..\\res\\skybox\\right.png",
		"..\\res\\skybox\\left.png",
		"..\\res\\skybox\\top.png",
		"..\\res\\skybox\\bottom.png",
		"..\\res\\skybox\\front.png",
		"..\\res\\skybox\\back.png"
	};

	skybox.init(faces);

	deltaTime.InitDeltaTime();
	kSpeed = 50.0f;

	keyboard_state_array = SDL_GetKeyboardState(NULL);
	engaging = false;

	missileCounter = 20;
	manualMissileEngage = true;

	rayCaster.initRayCaster(myCamera.getProjection(), myCamera.getView(), myCamera.getPos());

	GameInstructions();
}

void MainGame::GameInstructions()
{
	instructionManual =
		"\n"
		"WELCOME TO ADASTRA - A 3D ASTEROID REVISION \n" 
		"USE THE W-A-S-D KEYS TO MOVE THORUGH SPACE \n"
		"SPACE AND LEFT-SHIFT KEYS TO ADJUST ELEVATION \n"
		"WHEN THE SHIP IS STANDING STILL THE PERIFERAL VIEW CAN BE ENHANCED \n"
		"PRESS THE DOWN-ARROW KEY TO GET A BETTER GRASP OF THE NEAR-SPACE SITUATION \n"
		"\n"
		"TO FIRE A MISSILE CLICK THE LEFT-MOUSE BUTTON ON THE MOUSE DEVICE \n"
		"MISSILES CAN BE MANUALLY AIMED OR AUTOMATICALLY SEEK THEIR TARGET \n"
		"CLICK THE RIGHT-MOUSE BUTTON INSTEAD TO ENGAGE THE MISSILE HOMING-MODE \n"
		"FREELY SWITCH BETWEEN MODES BY USING THE SAME COMMAND \n"
		"\n"
		"HOMING MISSILES ARE EFFICIENT IF YOU ARE IN A SAFE SPOT \n"
		"MANUAL AIM BECOMES NECESSARY IN SPECIFIC OCCASIONS \n"
		"IF THE SITUATION IS TIGHT AND ACCURACY IS NEEDED THE LATTER CHOICE IS RECCOMENDED \n"
		"\n"
		"AVOID ASTEROIDS TO PREVENT THE SHIP DEMISE \n"
		"EARN THE PAY BY CLEARING THE SPACE FROM ALL THE ASTEROIDS \n"
		"\n"
	;

	cout << instructionManual;
}

void MainGame::createScreenQuad()
{
	float quadVertices[] = { 
		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		//positions   //texCoords
		-1.1f,  1.1f,  0.0f, 1.0f,
		-1.1f, -1.1f,  0.0f, 0.0f,
		 1.1f, -1.1f,  1.0f, 0.0f,

		 //Fixed white border on the right right-angled triangle by increasing the values
		-1.1f,  1.1f,  0.0f, 1.0f,
		 1.1f, -1.1f,  1.0f, 0.0f,
		 1.1f,  1.1f,  1.0f, 1.0f
	};

	// cube VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //stride offset example
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

}


void MainGame::gameLoop()
{
	while (_gameState != GameState::EXIT)
	{
		gameAudio.playMusic();

		processInput();
		drawGame();

		rayCaster.UpdateRay(myCamera.getView(), _gameDisplay.getWidth(), _gameDisplay.getHeight(), myCamera.getPos());

		deltaTime.UpdateDeltaTime();
		UpdateDeltaSpeed();

		counter += 2 * deltaTime.GetDeltaTime();
	}
}

void MainGame::processInput()
{
	SDL_Event evnt;

	SDL_PollEvent(&evnt);

	if (keyboard_state_array[SDL_SCANCODE_ESCAPE])
	{
		cout << "RETREAT-MODE ACTIVATED \n\n";

		cout << "WITHDRAWAL SUCCEEDED \n\n";

		_gameState = GameState::EXIT;
	}

	if (!keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_S] && !keyboard_state_array[SDL_SCANCODE_W])
	{
		engaging = false;

		if (keyboard_state_array[SDL_SCANCODE_DOWN] && (ship.getTM().GetPos()->z - myCamera.getPos().z) < 100)
		{
			myCamera.MoveBack(-deltaSpeed);
		}
	}

	if (!keyboard_state_array[SDL_SCANCODE_DOWN])
	{
		engaging = true;

		if ((evnt.type == SDL_KEYDOWN || evnt.type == SDL_KEYUP) || engaging)
		{
			if (keyboard_state_array[SDL_SCANCODE_SPACE] && (ship.getTM().GetPos()->y < 10) && !keyboard_state_array[SDL_SCANCODE_LSHIFT])
			{
				myCamera.setLook(glm::vec3(myCamera.getPos().x, (ship.getTM().GetPos()->y * -deltaTime.GetDeltaTime()) / 100, ship.getTM().GetPos()->z));
				ship.transformPositions(glm::vec3((*ship.getTM().GetPos() + glm::vec3(0, deltaSpeed, 0))), *ship.getTM().GetRot(), *ship.getTM().GetScale());
			}
			else if (keyboard_state_array[SDL_SCANCODE_LSHIFT] && (ship.getTM().GetPos()->y > - 10) && !keyboard_state_array[SDL_SCANCODE_SPACE])
			{
				myCamera.setLook(glm::vec3(myCamera.getPos().x, (ship.getTM().GetPos()->y * -deltaTime.GetDeltaTime()) / 100, ship.getTM().GetPos()->z));
				ship.transformPositions(glm::vec3((*ship.getTM().GetPos() - glm::vec3(0, deltaSpeed, 0))), *ship.getTM().GetRot(), *ship.getTM().GetScale());
			}

			if (keyboard_state_array[SDL_SCANCODE_A] && ship.getTM().GetPos()->x < 100)
			{
				ship.transformPositions(glm::vec3((*ship.getTM().GetPos() + glm::vec3(deltaSpeed, 0, 0))), glm::vec3(0, 0, 0), *ship.getTM().GetScale());
			}

			if (keyboard_state_array[SDL_SCANCODE_D] && ship.getTM().GetPos()->x > -100)
			{
				ship.transformPositions(glm::vec3((*ship.getTM().GetPos() - glm::vec3(deltaSpeed, 0, 0))), glm::vec3(0, 3.10, 0), *ship.getTM().GetScale());
			}

			if (ship.getTM().GetPos()->z > -100)
			{
				if (keyboard_state_array[SDL_SCANCODE_S] && !keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() - glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, 1.55, 0), *ship.getTM().GetScale());
				}
				else if (keyboard_state_array[SDL_SCANCODE_S] && keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() - glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, 0.75, 0), *ship.getTM().GetScale());
				}
				else if (keyboard_state_array[SDL_SCANCODE_S] && keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_A])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() - glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, 2.35, 0), *ship.getTM().GetScale());
				}
			}

			if (ship.getTM().GetPos()->z < 100)
			{
				if (keyboard_state_array[SDL_SCANCODE_W] && !keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() + glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, -1.55, 0), *ship.getTM().GetScale());
				}
				else if (keyboard_state_array[SDL_SCANCODE_W] && keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() + glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, -0.75, 0), *ship.getTM().GetScale());
				}
				else if (keyboard_state_array[SDL_SCANCODE_W] && keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_A])
				{
					ship.transformPositions(glm::vec3((*ship.getTM().GetPos() + glm::vec3(0, 0, deltaSpeed))), glm::vec3(0, -2.35, 0), *ship.getTM().GetScale());
				}
			}

			myCamera.setPos(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y + 20, ship.getTM().GetPos()->z - 50));

		}
	}

	if (evnt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (evnt.button.button == SDL_BUTTON_LEFT)
		{
			if (missileCounter > 0 && !missiles[missileCounter].getActive())
			{
				gameAudio.playSoundEffect(1);
				missileCounter--;
				fireMissiles(missileCounter);
			}
		}

		if (evnt.button.button == SDL_BUTTON_RIGHT)
		{
			gameAudio.playSoundEffect(2);

			manualMissileEngage = !manualMissileEngage;

			if (manualMissileEngage)
			{
				cout << "HOMING MISSILES DISENGAGED \n\n";
			}
			else
			{
				cout << "HOMING MISSILES ENGAGED \n\n";
			}
		}
	}
}

void MainGame::initModels(GameObject*& asteroid)
{
	ship.transformPositions(glm::vec3(-50.0, 0.0, -50.0), glm::vec3(0, -1.55, 0), glm::vec3(1.0, 1.0, 1.0));

	for (int i = 0; i < 20; ++i)
	{
		int range = 50 - -50.0 + 1;
		int rAsDir = rand() % range + -50.0;
		
		float rX = -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));
		float rY= -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));
		float rZ = -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));

		asteroid[i].transformPositions(glm::vec3(10.0 * i * rX, 2.0 * i * rY * 0, 10.0 * i * rZ), glm::vec3(rX, rY, rZ), glm::vec3(1.0, 1.0, 1.0));
		asteroid[i].update(&rockMesh);
		asDir[i] = rAsDir;
	}

	for (int i = 0; i < 20; ++i)
	{
		missiles[i].setActive(false);
	}
}

void MainGame::drawAsteriods()
{
	texture.Bind(0);
	eMapping.Bind();
	linkEmapping();

	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, texture.getID());

	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getID());

	for (int i = 0; i < 20; ++i)
	{
		if (asteroid[i].getActive())
		{
			if (collision(asteroid[i].getTM().GetPos(), 1.0, ship.getTM().GetPos(), 1.0))
			{
				asteroid[i].transformPositions(glm::vec3(1000, 1000, 1000), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
				asteroid[i].draw(&rockMesh);
				asteroid[i].update(&missileMesh);
				asteroid[i].setActive(false);
				ship.transformPositions(glm::vec3(0, 0.0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));

				cout << "SHIP OBLITERATED \n\n";

				cout << "NO PAY FOR THE DEAD \n\n";

				_gameState = GameState::EXIT;
			}

			asteroid[i].transformPositions(glm::vec3(asteroid[i].getTM().GetPos()->x + asDir[i] * deltaTime.GetDeltaTime(), 0, asteroid[i].getTM().GetPos()->z + asDir[i + 1] * deltaTime.GetDeltaTime()), glm::vec3(asteroid[i].getTM().GetRot()->x + deltaTime.GetDeltaTime(), asteroid[i].getTM().GetRot()->y + deltaTime.GetDeltaTime(), asteroid[i].getTM().GetRot()->z + deltaTime.GetDeltaTime()), glm::vec3(0.2, 0.2, 0.2));
			asteroid[i].draw(&rockMesh);
			asteroid[i].update(&rockMesh);
			eMapping.Update(asteroid[i].getTM(), myCamera);

			if ((asteroid[i].getTM().GetPos()->x) > 200 || (asteroid[i].getTM().GetPos()->x) < - 200)
			{
				asteroid[i].transformPositions(-*asteroid[i].getTM().GetPos(), glm::vec3(asteroid[i].getTM().GetRot()->x + deltaTime.GetDeltaTime(), asteroid[i].getTM().GetRot()->y + deltaTime.GetDeltaTime(), asteroid[i].getTM().GetRot()->z + deltaTime.GetDeltaTime()), glm::vec3(0.2, 0.2, 0.2));
			}
		}
	}
}

void MainGame::drawMissiles()
{
	texture.Bind(0);
	fogShader.Bind();
	linkFogShader();

	for (int i = 0; i < 20; ++i)
	{
		if (missiles[i].getActive())
		{
			for (int j = 0; j < 20; ++j)
			{
				if (collision(asteroid[j].getTM().GetPos(), 2.0, missiles[i].getTM().GetPos(), 2.0))
				{
					asteroid[j].transformPositions(glm::vec3(1000, 1000, 1000), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
					asteroid[j].draw(&rockMesh);
					asteroid[j].update(&rockMesh);
					asteroid[j].setActive(false);
					missiles[i].transformPositions(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
					missiles[i].draw(&missileMesh);
					missiles[i].update(&missileMesh);
					missiles[i].setActive(false);

					cout << "ASTEROID ANNIHILATED \n\n";

					if (i == 0)
					{
						cout << "MISSION ACCOMPLISHED \n\n";

						cout << "ALL ASTEROIDS HAVE BEEN DESTROYED \n\n";

						_gameState = GameState::EXIT;
					}
				}
			}

			if (manualMissileEngage)
			{
				//ManualHomingMissiles
				missiles[i].transformPositions(glm::vec3(glm::normalize(glm::vec3(rayCaster.GetCurrentPlanePoint().x, 0, rayCaster.GetCurrentPlanePoint().z) - *missiles[i].getTM().GetPos()) + glm::vec3(missiles[i].getTM().GetPos()->x + deltaTime.GetDeltaTime(), 0, missiles[i].getTM().GetPos()->z + deltaTime.GetDeltaTime())), glm::vec3(glm::normalize(*missiles[i].getTM().GetPos() + glm::vec3(rayCaster.GetCurrentPlanePoint().x, 0, rayCaster.GetCurrentPlanePoint().z)) + glm::normalize(*ship.getTM().GetPos() + *missiles[i].getTM().GetPos()) + glm::vec3(0, counter, 0)), *missiles[i].getTM().GetScale());
			}
			else
			{
				int j = 0;
				while (!asteroid[j].getActive())
				{
					j++;
				}

				//AutomaticHomingMissiles
				missiles[i].transformPositions(glm::vec3(glm::normalize(*asteroid[j].getTM().GetPos() - *missiles[i].getTM().GetPos()) + glm::vec3(missiles[i].getTM().GetPos()->x + deltaSpeed, 0, missiles[i].getTM().GetPos()->z + deltaSpeed)), glm::vec3(glm::normalize(*missiles[i].getTM().GetPos() + *asteroid[j].getTM().GetPos()) + glm::normalize(*ship.getTM().GetPos() + *missiles[i].getTM().GetPos()) + glm::vec3(0, 0, counter)), *missiles[i].getTM().GetScale());
			}
			missiles[i].draw(&missileMesh);
			missiles[i].update(&missileMesh);
			fogShader.Update(missiles[i].getTM(), myCamera);
		}
	}
}

void MainGame::fireMissiles(int missileNumber) 
{
	cout << "MISSILE FIRED \n\n";
	missiles[missileNumber].transformPositions(*ship.getTM().GetPos(), *ship.getTM().GetRot(), glm::vec3(0.3, 0.3, 0.3));
	missiles[missileNumber].setActive(true);
}

void MainGame::drawShip()
{
	texture.Bind(0);
	nShader.Bind();

	ship.draw(&shipMesh);
	ship.update(&shipMesh);
	nShader.Update(ship.getTM(), myCamera);
}


void MainGame::drawSkyBox()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);

	skybox.draw(&myCamera);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnd();
}


bool MainGame::collision(glm::vec3 *m1Pos, float m1Rad, glm::vec3 *m2Pos, float m2Rad)
{
	float distance = glm::sqrt((m2Pos->x - m1Pos->x)*(m2Pos->x - m1Pos->x) + (m2Pos->y - m1Pos->y)*(m2Pos->y - m1Pos->y) + (m2Pos->z - m1Pos->z)*(m2Pos->z - m1Pos->z));

	if (distance < (m1Rad + m2Rad))
	{
		cout << "IMPACT CONFIRMED \n\n";
		gameAudio.playSoundEffect(0);
		shake = true;
		return true;
	}
	else
	{
		return false;
	}
}

void MainGame::UpdateDeltaSpeed()
{
	deltaSpeed = kSpeed * deltaTime.GetDeltaTime();
}

void MainGame::linkFogShader()
{
	fogShader.setFloat("maxDist", 20.0f);
	fogShader.setFloat("minDist", 0.0f);
	fogShader.setVec3("fogColor", glm::vec3(0.0f, 0.0f, 0.0f));
}

void MainGame::linkEmapping()
{
	//Avoid shader issue by fetching an asteroid that is no longer active
	for (int i = 0; i < 20; ++i)
	{
		if (asteroid[i].getActive())
		{
			eMapping.setMat4("model", asteroid[i].getModel());
		}
	}
}

void MainGame::bindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
}

void MainGame::unbindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void MainGame::generateFBO(float w, float h)
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// create a colorbuffer for attachment texture
	glGenTextures(1, &CBO);
	glBindTexture(GL_TEXTURE_2D, CBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CBO, 0);

	// create a renderbuffer object for depth and stencil attachment 
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h); // use a single renderbuffer object for both a depth AND stencil buffer.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); // now actually attach it


	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		cout << "FRAMEBUFFER:: Framebuffer is complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MainGame::renderFBO()
{
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	FBOShader.Bind();
	if (shake)
	{
		FBOShader.setFloat("time", counter);

		if (((int)counter % 4) < 1)
		{
			shake = false;
		}
	}
	else
	{
		FBOShader.setFloat("time", 1);
	}

	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, CBO);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void MainGame::drawGame()
{
	_gameDisplay.clearDisplay(0.8f, 0.8f, 0.8f, 1.0f); //sets our background colour	

	bindFBO();

	drawAsteriods();
	drawShip();
	drawSkyBox();
	drawMissiles();

	unbindFBO();

	renderFBO();

	glEnable(GL_DEPTH_TEST);
	
	_gameDisplay.swapBuffer();		
} 

