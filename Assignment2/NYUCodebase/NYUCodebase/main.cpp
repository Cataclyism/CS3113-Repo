#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <random>
#include <ctime>


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.14159265359f

class Entity {
public:
	void Draw();
	Entity();

	float x;
	float y;
	float width;
	float height;
	float angle;

	bool reset;
};

Entity::Entity() {
	reset = false;
}

SDL_Window* displayWindow;

GLuint LoadTexture(const char*); 

void Render(ShaderProgram, GLuint, Entity&, Entity&, Entity&);

void GetRandAngle(float &);

void ProcessEvents(SDL_Event&, Entity&, Entity&, float, bool&);

void Update(Entity &, Entity &, Entity &, const float &);

// no DrawTexture declaration because I didn't want to fiddle with dynamic mem

void DrawTexture(ShaderProgram program, GLuint texture, float vertices[], float texCoords[]) {

	glBindTexture(GL_TEXTURE_2D, texture);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}




int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif


	glViewport(0, 0, 640, 360);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint whiteSpace = LoadTexture("whiteSpace.png");

	Matrix projectionMatrix;
	//Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;

	Entity Ball;
	Entity LeftBar;
	Entity RightBar;
	srand(time(0));
	Ball.angle = 90.0f * PI / 180.0f;
	GetRandAngle(Ball.angle);

	Ball.x = 0.0f;
	Ball.y = 0.0f;
	LeftBar.x = -3.05f;
	LeftBar.y = 0.0f;
	RightBar.x = 3.05f;
	RightBar.y = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {		
		
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);

		ProcessEvents(event, LeftBar, RightBar, elapsed, done);

		Update(Ball, LeftBar, RightBar, elapsed);

		Render(program, whiteSpace, Ball, LeftBar, RightBar);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SDL_FreeSurface(surface);

	return textureID;
}

void Render(ShaderProgram program, GLuint colorTexture, Entity &Ball, Entity &LeftBar, Entity &RightBar) {

	Matrix modelMatrix;



	modelMatrix.Translate(Ball.x, Ball.y, 0.0f);

	if (Ball.reset) {
		modelMatrix.setPosition(0.0f, 0.0f, 0.0f);
		Ball.reset = false;
	}

	program.setModelMatrix(modelMatrix);
	modelMatrix.identity();

	float verticesBall[] = { -0.1, -0.1, 0.1, -0.1, 0.1, 0.1, -0.1, -0.1, 0.1, 0.1, -0.1, 0.1 };

	float texCoordsBall[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	DrawTexture(program, colorTexture, verticesBall, texCoordsBall);

	modelMatrix.Translate(0.0f, LeftBar.y, 0.0f);

	program.setModelMatrix(modelMatrix);
	modelMatrix.identity();

	float verticesLeftBar[] = { -3.15, -0.4, -2.95, -0.4, -2.95, 0.4, -3.15, -0.4, -2.95, 0.4, -3.15, 0.4 };

	float texCoordsLeftBar[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	DrawTexture(program, colorTexture, verticesLeftBar, texCoordsLeftBar);

	modelMatrix.Translate(0.0f, RightBar.y, 0.0f);

	program.setModelMatrix(modelMatrix);
	modelMatrix.identity();

	float verticesRightBar[] = { 2.95, -0.4, 3.15, -0.4, 3.15, 0.4, 2.95, -0.4, 3.15, 0.4, 2.95, 0.4 };

	float texCoordsRightBar[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	DrawTexture(program, colorTexture, verticesRightBar, texCoordsRightBar);

	program.setModelMatrix(modelMatrix);

	float verticesUpBorder[] = { -3.55, 1.8, 3.55, 1.8, 3.55, 2.0, -3.55, 1.8, 3.55, 2.0, -3.55, 2.0 };

	float texCoordsUpBorder[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	DrawTexture(program, colorTexture, verticesUpBorder, texCoordsUpBorder);

	float verticesBotBorder[] = { -3.55, -2.0, 3.55, -2.0, 3.55, -1.8, -3.55, -2.0, 3.55, -1.8, -3.55, -1.8 };

	float texCoordsBotBorder[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	DrawTexture(program, colorTexture, verticesBotBorder, texCoordsBotBorder);
	
	float verticesDotLine[] = { -0.1, -1.7, 0.1, -1.7, 0.1, -1.5, -0.1, -1.7, 0.1, -1.5, -0.1, -1.5 };
	float texCoordsDotLine[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	for (unsigned int i = 0; i < 9; i++) {

		DrawTexture(program, colorTexture, verticesDotLine, texCoordsDotLine);

		verticesDotLine[1] += 0.4;
		verticesDotLine[3] += 0.4;
		verticesDotLine[5] += 0.4;
		verticesDotLine[7] += 0.4;
		verticesDotLine[9] += 0.4;
		verticesDotLine[11] += 0.4;

	}	
}

void GetRandAngle(float &angle) {
	//generates a random angle to lauch the ball in, but generated is in 60-120 or 240-300 range, retries
	//sometimes this doesn't work. when it doesn't work it seems to be in the 60-120 range
	do angle = ((float)rand() / (float)RAND_MAX) * 2 * PI;
	while (angle <= 120.0f * PI / 180.0f && angle >= 60.0f * PI / 180.0f || angle <= 300.0f * PI / 180.0f && angle >= 240.0f * PI / 180.0f);

}

void ProcessEvents(SDL_Event &event, Entity &LeftBar, Entity &RightBar, float elapsed, bool &done) {

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_UP] && (RightBar.y + 0.8 / 2) <= 1.8) RightBar.y += elapsed * 2.0f;
	if (keys[SDL_SCANCODE_DOWN] && (RightBar.y - 0.8 / 2) >= -1.8) RightBar.y -= elapsed * 2.0f;
	if (keys[SDL_SCANCODE_W] && (LeftBar.y + 0.8 / 2) <= 1.8) LeftBar.y += elapsed * 2.0f;
	if (keys[SDL_SCANCODE_S] && (LeftBar.y - 0.8 / 2) >= -1.8) LeftBar.y -= elapsed * 2.0f;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

	}
}

void Update(Entity &Ball, Entity &LeftBar, Entity &RightBar, const float &elapsed) {
	//collision or resetting when hitting the 4 walls

	if ((Ball.y + 0.2 / 2) >= 1.8) {
		Ball.angle = 2 * PI - Ball.angle;
	}
	if ((Ball.y - 0.2 / 2) <= -1.8) {
		Ball.angle = 2 * PI - Ball.angle;
	}
	if ((Ball.x + 0.2 / 2) >= 3.55) {
		Ball.reset = true;
		GetRandAngle(Ball.angle);
		Ball.x = 0.0f;
		Ball.y = 0.0f;
	}
	if ((Ball.x - 0.2 / 2) <= -3.55) {
		Ball.reset = true;
		GetRandAngle(Ball.angle);
		Ball.y = 0.0f;
		Ball.x = 0.0f;
	}
    //colision works fine except that all "bounces" are in the direction they came from. i.e ball from 300 deg bounces in a 60 deg direction off the right paddle

	if (!(Ball.y - 0.2 / 2 >= RightBar.y + 0.8 / 2 || Ball.y + .2 / 2 <= RightBar.y - 0.8 / 2 || Ball.x - 0.2 / 2 >= RightBar.x + 0.2 / 2 || Ball.x + 0.2 / 2 <= RightBar.x - 0.2 / 2)) {
		if (Ball.x - 0.2 / 2 <= RightBar.x + 0.2 / 2 || Ball.x + 0.2 / 2 >= RightBar.x - 0.2 / 2) {
			if (Ball.angle >= PI) Ball.angle = 3 * PI - Ball.angle;
			if (Ball.angle <= PI) Ball.angle = PI - Ball.angle;
		}
		if (Ball.y - 0.2 / 2 <= RightBar.y + 0.8 / 2 || Ball.y + 0.2 / 2 >= RightBar.y - 0.8 / 2) Ball.angle = 2 * PI - Ball.angle;

	}


	if (!(Ball.y - 0.2 / 2 >= LeftBar.y + 0.8 / 2 || Ball.y + .2 / 2 <= LeftBar.y - 0.8 / 2 || Ball.x - .2 / 2 >= LeftBar.x + 0.2 / 2 || Ball.x + .2 / 2 <= LeftBar.x - 0.2 / 2)) {
		if (Ball.x - 0.2 / 2 <= LeftBar.x + 0.2 / 2 || Ball.x + 0.2 / 2 >= LeftBar.x - 0.2 / 2) {
			if (Ball.angle >= PI) Ball.angle = 3 * PI - Ball.angle;
			if (Ball.angle <= PI) Ball.angle = PI - Ball.angle;
		}
		if (Ball.y - 0.2 / 2 <= LeftBar.y + 0.8 / 2 || Ball.y + 0.2 / 2 >= LeftBar.y - 0.8 / 2) Ball.angle = 2 * PI - Ball.angle;
	}

	Ball.x += cos(Ball.angle) * elapsed * 2.0f;
	Ball.y += sin(Ball.angle) * elapsed * 2.0f;

}