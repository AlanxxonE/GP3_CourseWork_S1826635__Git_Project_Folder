#pragma once
#include <vector>
#include <GL\glew.h>
#include "../GP3_CourseWork_S1826635/stb_image.h"
#include "../GP3_CourseWork_S1826635/Shader.h"
#include <iostream>
#include "../GP3_CourseWork_S1826635/Camera.h"

class Skybox
{
public:
	void init(std::vector<std::string> faces);
	void draw(Camera* camera);
	GLuint getID() { return textureID; }

	unsigned int textureID;
	unsigned int skyboxVAO;
	unsigned int skyboxVBO;

private:
	Shader shader;

};

