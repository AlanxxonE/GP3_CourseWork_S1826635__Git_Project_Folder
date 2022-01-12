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
	//whistle = audioDevice.loadSound("..\\res\\bang.wav");
	//backGroundMusic = audioDevice.loadSound("..\\res\\background.wav");
	texture.load("..\\res\\bricks.jpg");
	rockMesh.loadModel("..\\res\\Rock1.obj");
	shipMesh.loadModel("..\\res\\NewShip.obj");
	missileMesh.loadModel("..\\res\\NewR33.obj");
	fogShader.init("..\\res\\fogShader.vert", "..\\res\\fogShader.frag"); //new shader
	toonShader.init("..\\res\\shaderToon.vert", "..\\res\\shaderToon.frag"); //new shader
	rimShader.init("..\\res\\shaderRim.vert", "..\\res\\shaderRim.frag");
	eMapping.init("..\\res\\shaderReflection.vert", "..\\res\\shaderReflection.frag");
	FBOShader.init("..\\res\\FBOShader.vert", "..\\res\\FBOShader.frag");
	
	gameAudio.addAudioTrack("..\\res\\background.wav");
	gameAudio.addSoundEffect("..\\res\\whistle.wav");

	initModels(asteroid);

	geoShader.initGeo();

	myCamera.initCamera(glm::vec3(shipTransform.GetPos()->x, shipTransform.GetPos()->y + 20, -50), 70.0f, (float)_gameDisplay.getWidth()/_gameDisplay.getHeight(), 1.0f, 1000.0f);
	myCamera.setLook(glm::vec3(shipTransform.GetPos()->x, shipTransform.GetPos()->y, shipTransform.GetPos()->z));
	myCamera.setPos(glm::vec3(shipTransform.GetPos()->x, shipTransform.GetPos()->y + 20, shipTransform.GetPos()->z - 50));

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

	shipTransform.SetRot(glm::vec3(0, -1.55, 0));
	shipTransform.SetScale(glm::vec3(1.0, 1.0, 1.0));
	shipTransform.SetPos(glm::vec3(-80.0, 0.0, -80.0));
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

		//// vertex attributes for a quad that fills the half of the screen
		//-1.0f,  1.0f,  0.0f, 1.0f,
		//-1.0f,  0.25f,  0.0f, 0.0f,
		//-0.25f,  0.25f,  1.0f, 0.0f,

		//-1.0f,  1.0f,  0.0f, 1.0f,
		//-0.25f,  0.25f,  1.0f, 0.0f,
		//-0.25f,  1.0f,  1.0f, 1.0f
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
		//gameAudio.playMusic();
		processInput();
		currentCamPos = myCamera.getPos();
		drawGame();
		deltaTime.UpdateDeltaTime();
		UpdateDeltaSpeed();
		//updateDelta();
		//collision(rockMesh.getSpherePos(), rockMesh.getSphereRadius(), shipMesh.getSpherePos(), shipMesh.getSphereRadius());
		//playAudio(backGroundMusic, glm::vec3(0.0f,0.0f,0.0f));
		//cout << "x: " << myCamera.getPos().x << "y: " << myCamera.getPos().y << "z: " << myCamera.getPos().z << "\n";
	}
}

void MainGame::processInput()
{
	SDL_Event evnt;

	SDL_PollEvent(&evnt);

	if (!keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_S] && !keyboard_state_array[SDL_SCANCODE_W])
	{
		engaging = false;

		if (keyboard_state_array[SDL_SCANCODE_DOWN] && (shipTransform.GetPos()->z - currentCamPos.z) < 100)
		{
			myCamera.MoveBack(-deltaSpeed);
		}
	}

	if (!keyboard_state_array[SDL_SCANCODE_DOWN])
	{
		engaging = true;

		if ((evnt.type == SDL_KEYDOWN || evnt.type == SDL_KEYUP) || engaging)
		{
			if (keyboard_state_array[SDL_SCANCODE_SPACE] && (shipTransform.GetPos()->y < 10) && !keyboard_state_array[SDL_SCANCODE_LSHIFT])
			{
				myCamera.setLook(glm::vec3(myCamera.getPos().x, (shipTransform.GetPos()->y * -deltaTime.GetDeltaTime()) / 100, shipTransform.GetPos()->z));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(0,deltaSpeed,0))));
			}
			else if (keyboard_state_array[SDL_SCANCODE_LSHIFT] && (shipTransform.GetPos()->y > - 10) && !keyboard_state_array[SDL_SCANCODE_SPACE])
			{
				myCamera.setLook(glm::vec3(myCamera.getPos().x, (shipTransform.GetPos()->y * -deltaTime.GetDeltaTime()) / 100, shipTransform.GetPos()->z));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(0, deltaSpeed, 0))));
			}

			if (keyboard_state_array[SDL_SCANCODE_A])
			{
				shipTransform.SetRot(glm::vec3(0, 0, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(deltaSpeed, 0, 0))));
			}

			if (keyboard_state_array[SDL_SCANCODE_D])
			{
				shipTransform.SetRot(glm::vec3(0, 3.10, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(deltaSpeed, 0, 0))));
			}

			if (keyboard_state_array[SDL_SCANCODE_S] && !keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
			{
				shipTransform.SetRot(glm::vec3(0, 1.55, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(0, 0, deltaSpeed))));
			}
			else if (keyboard_state_array[SDL_SCANCODE_S] && keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
			{
				shipTransform.SetRot(glm::vec3(0, 0.75, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(0, 0, deltaSpeed))));
			}
			else if (keyboard_state_array[SDL_SCANCODE_S] && keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_A])
			{
				shipTransform.SetRot(glm::vec3(0, 2.35, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(0, 0, deltaSpeed))));
			}

			if (keyboard_state_array[SDL_SCANCODE_W] && !keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
			{
				shipTransform.SetRot(glm::vec3(0, -1.55, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(0, 0, deltaSpeed))));
			}
			else if (keyboard_state_array[SDL_SCANCODE_W] && keyboard_state_array[SDL_SCANCODE_A] && !keyboard_state_array[SDL_SCANCODE_D])
			{
				shipTransform.SetRot(glm::vec3(0, -0.75, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(0, 0, deltaSpeed))));
			}
			else if (keyboard_state_array[SDL_SCANCODE_W] && keyboard_state_array[SDL_SCANCODE_D] && !keyboard_state_array[SDL_SCANCODE_A])
			{
				shipTransform.SetRot(glm::vec3(0, -2.35, 0));
				shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(0, 0, deltaSpeed))));
			}

			myCamera.setPos(glm::vec3(shipTransform.GetPos()->x, shipTransform.GetPos()->y + 20, shipTransform.GetPos()->z - 50));

		}
	}

	if (evnt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (evnt.button.button == SDL_BUTTON_LEFT)
		{
			missileCounter--;
			fireMissiles(missileCounter);
		}
	}

	//while (SDL_PollEvent(&evnt)) //get and process events
	//{
	//	switch (evnt.type)
	//	{
	//	case SDL_MOUSEWHEEL:
	//		myCamera.MoveBack(evnt.wheel.y);
	//		break;
	//	default:
	//		break;
	//	case SDL_MOUSEBUTTONDOWN:
	//		switch (evnt.button.button)
	//		{
	//		case SDL_BUTTON_LEFT:
	//			//SDL_ShowSimpleMessageBox(0, "Mouse", "Left button was pressed!", _gameDisplay.getWindow());
	//			break;
	//		case SDL_BUTTON_RIGHT:
	//			//SDL_ShowSimpleMessageBox(0, "Mouse", "Right button was pressed!", _gameDisplay.getWindow());
	//			break;
	//		case SDL_BUTTON_MIDDLE:
	//			break;
	//		default:
	//			//SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
	//			break;
	//		}
	//	case SDL_KEYDOWN:
	//		/* Check the SDLKey values and move change the coords */
	//		switch (evnt.key.keysym.sym)
	//		{
	//		case SDLK_a:
	//			shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(deltaSpeed, 0, 0))));
	//			//transform.SetPos(glm::vec3(transform.GetPos()->x + 1.0f/* *deltaTime */, transform.GetPos()->y, 0.0));
	//			//cout << myCamera.getPos().x;
	//			break;
	//		case SDLK_w:
	//			shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() + glm::vec3(0,0,deltaSpeed))));
	//			//transform.SetPos(glm::vec3(transform.GetPos()->x, transform.GetPos()->y + 1.0f/* *deltaTime */, 0.0));
	//			break;
	//		case SDLK_s:
	//			shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(0, 0, deltaSpeed))));
	//			//transform.SetPos(glm::vec3(transform.GetPos()->x, transform.GetPos()->y - 1.0f /* *deltaTime */, 0.0));
	//			break;
	//		case SDLK_d:
	//			shipTransform.SetPos(glm::vec3((*shipTransform.GetPos() - glm::vec3(deltaSpeed, 0, 0))));
	//			//transform.SetPos(glm::vec3(transform.GetPos()->x - 1.0f*deltaTime, transform.GetPos()->y, 0.0));
	//			break;
	//		case SDLK_LEFT:
	//			myCamera.MoveLeft(-deltaSpeed);
	//			//cout << myCamera.getPos().x;
	//			break;
	//		case SDLK_RIGHT:
	//			myCamera.MoveRight(-deltaSpeed);
	//			break;
	//		case SDLK_UP:
	//			myCamera.MoveUp(deltaSpeed);
	//			break;
	//		case SDLK_DOWN:
	//			myCamera.MoveDown(deltaSpeed);
	//			break;
	//		case SDLK_SPACE:
	//			if (look)
	//				look = false;
	//			else
	//				look = true;
	//			break;
	//		case SDLK_BACKSPACE:
	//			if (shake)
	//				shake = false;
	//			else
	//				shake = true;
	//			break;
	//		default:
	//			break;
	//		case SDL_QUIT:
	//			_gameState = GameState::EXIT;
	//			break;
	//		}
	//	}

	//}
}

void MainGame::initModels(GameObject*& asteroid)
{
	for (int i = 0; i < 20; ++i)
	{
		int range = 20 - -20.0 + 1;
		int rAsDir = rand() % range + -20.0;
		float rX = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0)));
		float rY= -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));
		float rZ = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0)));

		asteroid[i].transformPositions(glm::vec3(10.0 * i * rX, 2.0 * i * rY * 0, 10.0 * i * rZ), glm::vec3(rX, rY, rZ), glm::vec3(1.0, 1.0, 1.0));
		asteroid[i].update(&rockMesh);
		asDir[i] = rAsDir;
	}

	//ship.transformPositions(glm::vec3(0.0, 0.0, -3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.2,0.2,0.2));
	
	for (int i = 0; i < 20; ++i)
	{
		//missilesTransform[i] = missiles[i].getTM();
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
			if (collision(asteroid[i].getTM().GetPos(), 1.0, shipTransform.GetPos(), 1.0))
			{
				asteroid[i].transformPositions(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
				asteroid[i].setActive(false);
				shipTransform.SetRot(glm::vec3(0, 0, 0));
				shipTransform.SetScale(glm::vec3(0, 0, 0));
				shipTransform.SetPos(glm::vec3(0, 0.0, 0));

				//_gameState = GameState::EXIT;
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
			if (collision(asteroid[i].getTM().GetPos(), 1.0, missiles[i].getTM().GetPos(), 1.0))
			{
				asteroid[i].transformPositions(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
				asteroid[i].setActive(false);
				missiles[i].transformPositions(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
				missiles[i].setActive(false);
			}

			//missilesTransform[i].SetPos(glm::vec3((missilesTransform[i].GetPos()->x + (missilesTransform[i].GetPos()->x - asteroid[i].getTM().GetPos()->x)) / sqrt((missilesTransform[i].GetPos()->x - asteroid[i].getTM().GetPos()->x)* (missilesTransform[i].GetPos()->x - asteroid[i].getTM().GetPos()->x) + (missilesTransform[i].GetPos()->z - asteroid[i].getTM().GetPos()->z)*(missilesTransform[i].GetPos()->z - asteroid[i].getTM().GetPos()->z)) + deltaSpeed,0, (missilesTransform[i].GetPos()->z + (missilesTransform[i].GetPos()->z - asteroid[i].getTM().GetPos()->z)) / sqrt((missilesTransform[i].GetPos()->x - asteroid[i].getTM().GetPos()->x) * (missilesTransform[i].GetPos()->x - asteroid[i].getTM().GetPos()->x) + (missilesTransform[i].GetPos()->z - asteroid[i].getTM().GetPos()->z) * (missilesTransform[i].GetPos()->z - asteroid[i].getTM().GetPos()->z)) + deltaSpeed));
			//missilesTransform[i].SetPos(glm::vec3((missilesTransform[i].GetPos()->x + asteroid[i].getTM().GetPos()->x) * 0.01,0, (missilesTransform[i].GetPos()->z + asteroid[i].getTM().GetPos()->z) * 0.01));
			//missilesTransform[i].SetRot(glm::vec3(1.55, counter, 0));
			
			//missilesTransform[i].SetPos(glm::vec3(glm::normalize(*asteroid[i].getTM().GetPos() - *missilesTransform[i].GetPos()) + glm::vec3(missilesTransform[i].GetPos()->x + deltaSpeed, 0, missilesTransform[i].GetPos()->z + deltaSpeed)));
			//missilesTransform[i].SetRot(glm::vec3(glm::normalize(*missilesTransform[i].GetPos() + *asteroid[i].getTM().GetPos()) + glm::normalize(*shipTransform.GetPos() + *missilesTransform[i].GetPos())));

			missiles[i].transformPositions(glm::vec3(glm::normalize(*asteroid[i].getTM().GetPos() - *missiles[i].getTM().GetPos()) + glm::vec3(missiles[i].getTM().GetPos()->x + deltaSpeed, 0, missiles[i].getTM().GetPos()->z + deltaSpeed)), glm::vec3(glm::normalize(*missiles[i].getTM().GetPos() + *asteroid[i].getTM().GetPos()) + glm::normalize(*shipTransform.GetPos() + *missiles[i].getTM().GetPos())), *missiles[i].getTM().GetScale());

			//missilesTransform[i].SetRot(glm::vec3(missilesTransform[i].GetPos()->x, counter, missilesTransform[i].GetPos()->z));
			//missiles[i].transformPositions(glm::vec3(*missilesTransform[i].GetPos()), glm::vec3(missilesTransform->GetRot()->x + deltaTime.GetDeltaTime(), missilesTransform->GetRot()->y + deltaTime.GetDeltaTime(), missilesTransform->GetRot()->z + deltaTime.GetDeltaTime()), glm::vec3(0.2, 0.2, 0.2));
			missiles[i].draw(&missileMesh);
			missiles[i].update(&missileMesh);
			fogShader.Update(missiles[i].getTM(), myCamera);
		}
	}
}

void MainGame::fireMissiles(int missileNumber) 
{
	SDL_PumpEvents();
	SDL_GetMouseState(&mouseX, &mouseY);

	//const float* mvSource = (const float*)glm::value_ptr(shipTransform.GetModel());
	//
	//for (int i = 0; i < 16; ++i)
	//{
	//	mvmatrix[i] = mvSource[i];
	//}

	//GLdouble mvMatrix[16];
	//GLdouble projMatrix[16];
	//GLint viewport[4];
	//GLdouble objx;
	//GLdouble objy;
	//GLdouble objz;
	//int realY;
	//glGetDoublev(GL_MODELVIEW_MATRIX, mvMatrix);
	//glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	//glGetIntegerv(GL_VIEWPORT, viewport);

	//cout << '\n';
	//cout << "VIEWPORT MATRIX" << '\n';;
	//cout << viewport[0] << "|" << viewport[1] << '\n';
	//cout << viewport[2] << "|" << viewport[3] << '\n';
	//cout << '\n';

	//cout << '\n';
	//cout << "MODELVIEW MATRIX" << '\n';;
	//cout << mvMatrix[0] << "|" << mvMatrix[1] << mvMatrix[2] << "|" << mvMatrix[3] << '\n';
	//cout << mvMatrix[4] << "|" << mvMatrix[5] << mvMatrix[6] << "|" << mvMatrix[7] << '\n';
	//cout << mvMatrix[8] << "|" << mvMatrix[9] << mvMatrix[10] << "|" << mvMatrix[11] << '\n';
	//cout << mvMatrix[12] << "|" << mvMatrix[13] << mvMatrix[14] << "|" << mvMatrix[15] << '\n';
	//cout << '\n';

	//cout << '\n';
	//cout << "PROJECTION MATRIX" << '\n';;
	//cout << projMatrix[0] << "|" << projMatrix[1] << projMatrix[2] << "|" << projMatrix[3] << '\n';
	//cout << projMatrix[4] << "|" << projMatrix[5] << projMatrix[6] << "|" << projMatrix[7] << '\n';
	//cout << projMatrix[8] << "|" << projMatrix[9] << projMatrix[10] << "|" << projMatrix[11] << '\n';
	//cout << projMatrix[12] << "|" << projMatrix[13] << projMatrix[14] << "|" << projMatrix[15] << '\n';
	//cout << '\n';

	//realY = viewport[3] - mouseY - 1;

	//const float* mvSource = (const float*)glm::value_ptr(shipTransform.GetModel());
	//
	//for (int i = 0; i < 16; ++i)
	//{
	//	mvMatrix[i] = mvSource[i];
	//}
	//gluUnProject((GLdouble)1000.0, (GLdouble)500.0, (GLdouble)1.0, mvMatrix, projMatrix, viewport, &objx, &objy, &objz);

	//cout << "X : " << (GLdouble)mouseX << "|||";
	//cout << "Y : " << (GLdouble)mouseY << "|||";


	//missilesTransform[missileNumber].SetScale(glm::vec3(0.3, 0.3, 0.3));
	//missilesTransform[missileNumber].SetPos(*shipTransform.GetPos());
	//missilesTransform[missileNumber].SetRot(*shipTransform.GetRot());
	missiles[missileNumber].transformPositions(*shipTransform.GetPos(), *shipTransform.GetRot(), glm::vec3(0.3, 0.3, 0.3));
	missiles[missileNumber].setActive(true);
	/** CALL THIS FROM processInput()
	* Set the missle transform to the ship transform ONCE (initial conditions)
	* Set the missle to active (check it is not already active)
	* Update the tranform to move the missle along its forward vector(more advanced, use seek to target)
	* check for asteroid collision
	* handle asteroid collision
	*/
}

void MainGame::drawShip()
{
	rimShader.Bind();
	linkRimLighting();

	ship.draw(&shipMesh);
	ship.update(&shipMesh);
	rimShader.Update(shipTransform, myCamera);
}


void MainGame::drawSkyBox()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);

	counter = counter + 0.02f;

	skybox.draw(&myCamera);

	myCamera.setPos(currentCamPos);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnd();
}


bool MainGame::collision(glm::vec3 *m1Pos, float m1Rad, glm::vec3 *m2Pos, float m2Rad)
{
	float distance = glm::sqrt((m2Pos->x - m1Pos->x)*(m2Pos->x - m1Pos->x) + (m2Pos->y - m1Pos->y)*(m2Pos->y - m1Pos->y) + (m2Pos->z - m1Pos->z)*(m2Pos->z - m1Pos->z));

	if (distance < (m1Rad + m2Rad))
	{
		gameAudio.playSoundEffect(0);
		cout << "COLLISION";
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
	//cout << "Delta Speed: " << deltaSpeed << "\n";
}

//void MainGame::playAudio(unsigned int Source, glm::vec3 pos)
//{
//	
//	ALint state; 
//	alGetSourcei(Source, AL_SOURCE_STATE, &state);
//	/*
//	Possible values of state
//	AL_INITIAL
//	AL_STOPPED
//	AL_PLAYING
//	AL_PAUSED
//	*/
//	if (AL_PLAYING != state)
//	{
//		audioDevice.playSound(Source, pos);
//	}
//}

void MainGame::linkFogShader()
{
	//fogShader.setMat4("u_pm", myCamera.getProjection());
	//fogShader.setMat4("u_vm", myCamera.getProjection());
	fogShader.setFloat("maxDist", 20.0f);
	fogShader.setFloat("minDist", 0.0f);
	fogShader.setVec3("fogColor", glm::vec3(0.0f, 0.0f, 0.0f));
}

void MainGame::linkToon()
{
	toonShader.setVec3("lightDir", glm::vec3(0.5f, 0.5f, 0.5f));
}

void MainGame::linkGeo()
{
	float randX = ((float)rand() / (RAND_MAX));
	float randY = ((float)rand() / (RAND_MAX));
	float randZ = ((float)rand() / (RAND_MAX));
	// Frag: uniform float randColourX; uniform float randColourY; uniform float randColourZ;
	geoShader.setFloat("randColourX", randX);
	geoShader.setFloat("randColourY", randY);
	geoShader.setFloat("randColourZ", randZ);
	// Geom: uniform float time;
	geoShader.setFloat("time", counter);
}

void MainGame::linkRimLighting()
{
	glm::vec3 camDir;
	camDir = shipMesh.getSpherePos() - myCamera.getPos();
	camDir = glm::normalize(camDir);
	rimShader.setMat4("u_pm", myCamera.getProjection());
	rimShader.setMat4("u_vm", myCamera.getView());
	rimShader.setMat4("model", transform.GetModel());
	rimShader.setMat4("view", myCamera.getView());
	rimShader.setVec3("lightDir", glm::vec3(0.5f, 0.5f, 0.5f));
}

void MainGame::linkEmapping()
{
	eMapping.setMat4("model", asteroid[0].getModel());
	//eMapping.setVec3("cameraPos", myCamera.getPos());
}

//void MainGame::updateDelta()
//{
//	LAST = NOW;
//	NOW = SDL_GetPerformanceCounter();
//
//	deltaTime = (float)((NOW - LAST) / (float)SDL_GetPerformanceFrequency());
//}

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
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
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

	//drawAsteriods();
	//drawShip();
	//drawSkyBox();
	//drawMissiles();
	
	_gameDisplay.swapBuffer();		
} 

