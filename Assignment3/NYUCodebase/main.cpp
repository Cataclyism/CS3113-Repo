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
#include <vector>


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.14159265359f

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

int state = STATE_MAIN_MENU;



class UnevenSheetSprite {
public:
	UnevenSheetSprite();
	UnevenSheetSprite(GLuint, float, float, float, float, float);

	void Draw(ShaderProgram *program);

	float size;
	GLuint textureID;
	float u;
	float v;
	float width;
	float height;
};

UnevenSheetSprite::UnevenSheetSprite() {
	//reset = false;
}


UnevenSheetSprite::UnevenSheetSprite(GLuint tID, float x, float y, float w, float h, float s) {
	textureID = tID;
	u = x;
	v = y;
	width = w;
	height = h;
	size = s;
}

class EvenSheetSprite {
public:
	EvenSheetSprite();
	EvenSheetSprite(GLuint, int, int, int, float);

	void Draw(ShaderProgram *program);

	float size;

	GLuint textureID;

	int index;
	int x;
	int y;

};

EvenSheetSprite::EvenSheetSprite() {
	//reset = false;
}

EvenSheetSprite::EvenSheetSprite(GLuint tID, int ind, int u, int v, float s) {
	textureID = tID;
	index = ind;
	x = u;
	y = v;
	size = s;
}

class Entity {
public:
	void Draw(ShaderProgram *program);
	Entity();

	float x;
	float y;
	float width;
	float height;
	float angle;
	float velocityX;
	float velocityY;


	bool inUse;

	bool unevenEven;

	bool EvenOrUneven() const {
		return (eImage.size <= 1.0f && eImage.size >= 0.0f);
	}

	UnevenSheetSprite uImage;
	EvenSheetSprite eImage;

};

Entity::Entity() { 
	x = 0.0f;
	y = 0.0f;
	width = 0.0f;
	height = 0.0f;
	angle = 0.0f;
	velocityX = 0.0;
	velocityY = 0.0;


	inUse = false; 
}


SDL_Window* displayWindow;

GLuint LoadTexture(const char*);

void Render(ShaderProgram, std::vector<std::vector<Entity>> &, std::vector<Entity> &, Entity &,
	std::vector<Entity> &, std::vector<Entity> &, std::vector<Entity> &, std::vector<Entity> &);

void GetRandAngle(float &);

void ProcessEvents(SDL_Event &event, std::vector<std::vector<Entity>> &, std::vector<Entity> &, Entity &, std::vector<Entity> &, float , bool &);

void Update(std::vector<std::vector<Entity>> &, std::vector<Entity> &, std::vector<Entity> &, Entity &, const float &);

ShaderProgram& Setup(Matrix &, std::vector<Entity> &, std::vector<Entity> &,
	std::vector<std::vector<Entity>> &, std::vector<Entity> &, std::vector<Entity> &, std::vector<Entity> &, Entity &);

// no DrawTexture declaration because I didn't want to fiddle with dynamic mem

//void DrawTexture(ShaderProgram program, GLuint texture, float vertices[], float texCoords[]) {
//
//	glBindTexture(GL_TEXTURE_2D, texture);
//
//	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
//	glEnableVertexAttribArray(program.positionAttribute);
//
//	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
//	glEnableVertexAttribArray(program.texCoordAttribute);
//
//	glDrawArrays(GL_TRIANGLES, 0, 6);
//
//	glDisableVertexAttribArray(program.positionAttribute);
//	glDisableVertexAttribArray(program.texCoordAttribute);
//
//}




int main(int argc, char *argv[]) {

	std::vector<Entity> playerProjectiles;
	std::vector<Entity> enemyProjectiles;
	std::vector<std::vector<Entity>> enemies;
	std::vector<Entity> startGame;
	std::vector<Entity> gameName;
	std::vector<Entity> gameOver;
	Entity player;

	srand(time(0));

	Matrix projectionMatrix;
	Matrix viewMatrix;
	Matrix modelMatrix;

	ShaderProgram program = Setup(projectionMatrix, playerProjectiles, enemyProjectiles, enemies, startGame, gameName, gameOver, player);

	float lastFrameTicks = 0.0f;

	enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
	int state;

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);

		ProcessEvents(event, enemies, enemyProjectiles, player, playerProjectiles, elapsed, done);

		Update(enemies, playerProjectiles, enemyProjectiles, player, elapsed);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		Render(program, enemies, enemyProjectiles, player, playerProjectiles, gameName, startGame, gameOver);



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

ShaderProgram& Setup(Matrix &projectionMatrix, std::vector<Entity> &playerProjectiles, std::vector<Entity> &enemyProjectiles, 
	std::vector<std::vector<Entity>> &enemies, std::vector<Entity> &startGame, std::vector<Entity> &gameName, 
	std::vector<Entity> &gameOver, Entity &player) {

	//normal SDL setup

	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif


	glViewport(0, 0, 640, 360);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//Enable Blending

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load the 2 sprite sheets

	GLuint gameSheetTexture = LoadTexture("sheet.png");
	GLuint fontSheetTexture = LoadTexture("pixel_font.png");

	// Load the game sprites

	UnevenSheetSprite en1(gameSheetTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.75f);
	UnevenSheetSprite en2(gameSheetTexture, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 1.0f);
	UnevenSheetSprite en3(gameSheetTexture, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 1.0f);
	UnevenSheetSprite en4(gameSheetTexture, 425.0f / 1024.0f, 384.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 1.0f);
	UnevenSheetSprite en5(gameSheetTexture, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 1.0f);
	UnevenSheetSprite en6(gameSheetTexture, 143.0f / 1024.0f, 293.0f / 1024.0f, 103.0f / 1024.0f, 84.0f / 1024.0f, 1.0f);
	UnevenSheetSprite enProjectile(gameSheetTexture, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2f);
	UnevenSheetSprite pla(gameSheetTexture, 211.0f / 1024.0f, 941.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.75f);
	UnevenSheetSprite plaProjectile(gameSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2f);

	// Load the Letter sprites, only loads needed letters/symbols

	EvenSheetSprite upA(fontSheetTexture, 4 * 16 + 1, 16, 16, 1.0f);
	EvenSheetSprite upB(fontSheetTexture, 4 * 16 + 2, 16, 16, 0.4f);
	EvenSheetSprite upC(fontSheetTexture, 4 * 16 + 3, 16, 16, 1.0f);
	EvenSheetSprite upD(fontSheetTexture, 4 * 16 + 4, 16, 16, 1.0f);
	EvenSheetSprite upE(fontSheetTexture, 4 * 16 + 5, 16, 16, 1.0f);
	EvenSheetSprite upG(fontSheetTexture, 4 * 16 + 7, 16, 16, 1.0f);
	EvenSheetSprite upI(fontSheetTexture, 4 * 16 + 9, 16, 16, 1.0f);
	EvenSheetSprite upL(fontSheetTexture, 4 * 16 + 12, 16, 16, 1.0f);
	EvenSheetSprite upM(fontSheetTexture, 4 * 16 + 13, 16, 16, 1.0f);
	EvenSheetSprite upN(fontSheetTexture, 4 * 16 + 14, 16, 16, 1.0f);
	EvenSheetSprite upO(fontSheetTexture, 4 * 16 + 15, 16, 16, 1.0f);
	EvenSheetSprite upP(fontSheetTexture, 5 * 16 + 0, 16, 16, 1.0f);
	EvenSheetSprite upR(fontSheetTexture, 5 * 16 + 2, 16, 16, 1.0f);
	EvenSheetSprite upS(fontSheetTexture, 5 * 16 + 3, 16, 16, 1.0f);
	EvenSheetSprite upT(fontSheetTexture, 5 * 16 + 4, 16, 16, 1.0f);
	EvenSheetSprite upV(fontSheetTexture, 5 * 16 + 6, 16, 16, 1.0f);
	EvenSheetSprite upY(fontSheetTexture, 5 * 16 + 9, 16, 16, 1.0f);

	EvenSheetSprite loA(fontSheetTexture, 6 * 16 + 1, 16, 16, 1.0f);
	EvenSheetSprite loC(fontSheetTexture, 6 * 16 + 3, 16, 16, 1.0f);
	EvenSheetSprite loE(fontSheetTexture, 6 * 16 + 5, 16, 16, 1.0f);
	EvenSheetSprite loO(fontSheetTexture, 6 * 16 + 15, 16, 16, 1.0f);
	EvenSheetSprite loP(fontSheetTexture, 7 * 16 + 0, 16, 16, 1.0f);
	EvenSheetSprite loR(fontSheetTexture, 7 * 16 + 2, 16, 16, 1.0f);
	EvenSheetSprite loS(fontSheetTexture, 7 * 16 + 3, 16, 16, 1.0f);
	EvenSheetSprite loT(fontSheetTexture, 7 * 16 + 4, 16, 16, 1.0f);

	EvenSheetSprite exclaim(fontSheetTexture, 2 * 16 + 1, 16, 16, 1.0f);
	EvenSheetSprite space(fontSheetTexture, 2 * 16 + 0, 16, 16, 1.0f);

	//vectors of sprites

	std::vector<UnevenSheetSprite> unevenEnemies;
	std::vector<EvenSheetSprite> evenLetters;

	unevenEnemies.push_back(en1);
	unevenEnemies.push_back(en2);
	unevenEnemies.push_back(en3);
	unevenEnemies.push_back(en4);
	unevenEnemies.push_back(en5);
	unevenEnemies.push_back(en6);

	evenLetters.push_back(upA);
	evenLetters.push_back(upB);
	evenLetters.push_back(upC);
	evenLetters.push_back(upD);
	evenLetters.push_back(upE);
	evenLetters.push_back(upG);
	evenLetters.push_back(upI);
	evenLetters.push_back(upL);
	evenLetters.push_back(upM);
	evenLetters.push_back(upN);
	evenLetters.push_back(upO);
	evenLetters.push_back(upP);
	evenLetters.push_back(upR);
	evenLetters.push_back(upS);
	evenLetters.push_back(upT);
	evenLetters.push_back(upV);
	evenLetters.push_back(upY);

	evenLetters.push_back(loA);
	evenLetters.push_back(loC);
	evenLetters.push_back(loE);
	evenLetters.push_back(loO);
	evenLetters.push_back(loP);
	evenLetters.push_back(loR);
	evenLetters.push_back(loS);
	evenLetters.push_back(loT);

	evenLetters.push_back(exclaim);
	evenLetters.push_back(space);

	// title screen setup
	// used sprite letter vector to make a entity letter vector

	std::vector<Entity> letters;

	for (unsigned int i = 0; i < evenLetters.size(); ++i) {
		Entity letter;
		letter.eImage = evenLetters[i];

		letters.push_back(letter);
	}

	gameName.push_back(letters[14]);	//T
	gameName.push_back(letters[10]);	//O
	gameName.push_back(letters[14]);	//T
	gameName.push_back(letters[0]);		//A
	gameName.push_back(letters[7]);		//L
	gameName.push_back(letters[7]);		//L
	gameName.push_back(letters[16]);	//Y
	gameName.push_back(letters[26]);	//
	gameName.push_back(letters[9]);		//N
	gameName.push_back(letters[10]);	//O
	gameName.push_back(letters[14]);	//T
	gameName.push_back(letters[26]);	//
	gameName.push_back(letters[13]);	//S
	gameName.push_back(letters[11]);	//P
	gameName.push_back(letters[0]);		//A
	gameName.push_back(letters[2]);		//C
	gameName.push_back(letters[4]);		//E
	gameName.push_back(letters[26]);	//
	gameName.push_back(letters[6]);		//I
	gameName.push_back(letters[9]);		//N
	gameName.push_back(letters[15]);	//V
	gameName.push_back(letters[0]);		//A
	gameName.push_back(letters[3]);		//D
	gameName.push_back(letters[4]);		//E
	gameName.push_back(letters[12]);	//R
	gameName.push_back(letters[13]);	//S

	for (unsigned int i = 0; i < gameName.size(); ++i) {
		gameName[i].x = -3.15f + i * 0.25f;
		gameName[i].y = 0.5f;
		gameName[i].eImage.size = 0.25f;
	}

	startGame.push_back(letters[11]);	//P
	startGame.push_back(letters[22]);	//r
	startGame.push_back(letters[19]);	//e
	startGame.push_back(letters[23]);	//s
	startGame.push_back(letters[23]);	//s
	startGame.push_back(letters[26]);	//
	startGame.push_back(letters[13]);	//S
	startGame.push_back(letters[21]);	//p
	startGame.push_back(letters[17]);	//a
	startGame.push_back(letters[18]);	//c
	startGame.push_back(letters[19]);	//e
	startGame.push_back(letters[26]);	//
	startGame.push_back(letters[1]);	//B
	startGame.push_back(letters[17]);	//a
	startGame.push_back(letters[22]);	//r
	startGame.push_back(letters[26]);	//
	startGame.push_back(letters[24]);	//t
	startGame.push_back(letters[20]);	//o
	startGame.push_back(letters[26]);	//
	startGame.push_back(letters[13]);	//S
	startGame.push_back(letters[24]);	//t
	startGame.push_back(letters[17]);	//a
	startGame.push_back(letters[22]);	//r
	startGame.push_back(letters[24]);	//t
	startGame.push_back(letters[25]);	//!

	for (unsigned int i = 0; i < startGame.size(); ++i) {
		startGame[i].x = -2.4f + i * 0.2f;
		startGame[i].y = -0.5f;
		startGame[i].eImage.size = 0.2f;
	}

	// game end state setup

	gameOver.push_back(letters[5]);		//G
	gameOver.push_back(letters[0]);		//A
	gameOver.push_back(letters[8]);		//M
	gameOver.push_back(letters[4]);		//E
	gameOver.push_back(letters[26]);	// 
	gameOver.push_back(letters[10]);	//O
	gameOver.push_back(letters[15]);	//V
	gameOver.push_back(letters[4]);		//E
	gameOver.push_back(letters[12]);	//R

	for (unsigned int i = 0; i < gameOver.size(); ++i) {
		gameOver[i].x = -2.0f + i * 0.5f;
		gameOver[i].y = 0.0f;
		gameOver[i].eImage.size = 0.5f;
	}

	// initial game state setup

	for (unsigned int i = 0; i < 100; ++i) {
		Entity plaProj;
		plaProj.uImage = plaProjectile;
		plaProj.x = 4.0f + i * 0.3f;
		plaProj.y = 1.0f;
		plaProj.uImage.size = 0.2f;

		playerProjectiles.push_back(plaProj);
	}

	for (unsigned int i = 0; i < 200; ++i) {
		Entity enProj;
		enProj.uImage = enProjectile;
		enProj.x = 4.0f + i * 0.3f;
		enProj.y = 0.0f;
		enProj.uImage.size = 0.2f;
		enProj.angle = 180 * PI / 180;

		enemyProjectiles.push_back(enProj);
	}

	for (unsigned int i = 0; i < 6; ++i) {
		std::vector<Entity> row;

		for (unsigned int j = 0; j < 12; ++j) {
			Entity ene;
			ene.uImage = unevenEnemies[i];
			ene.x = -2.75f + j * 0.5f;
			ene.y = -0.5f + i * 0.45f;
			ene.uImage.size = 0.35f;
			if (i % 2 == 0) ene.velocityX = -0.2;
			else ene.velocityX = 0.2;
			ene.inUse = true;

			row.push_back(ene);
		}
		enemies.push_back(row);
	}

	player.uImage = pla;
	player.x = 0.0f;
	player.y = -1.4f;
	player.uImage.size = 0.35f;


	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	return program;
}

void Render(ShaderProgram program, std::vector<std::vector<Entity>> &enemies, std::vector<Entity> &enemyProjectiles, Entity &player,
	std::vector<Entity> &playerProjectiles, std::vector<Entity> &gameName, std::vector<Entity> &startGame, std::vector<Entity> &gameOver) {

	if (state == STATE_MAIN_MENU) {
		for (unsigned int i = 0; i < gameName.size(); ++i) gameName[i].Draw(&program);

		for (unsigned int i = 0; i < startGame.size(); ++i) startGame[i].Draw(&program);
	}



	if (state == STATE_GAME_LEVEL) {
		for (unsigned int i = 0; i < playerProjectiles.size(); ++i) playerProjectiles[i].Draw(&program);

		for (unsigned int i = 0; i < enemyProjectiles.size(); ++i) enemyProjectiles[i].Draw(&program);

		for (unsigned int i = 0; i < enemies.size(); ++i) {
			for (unsigned int j = 0; j < enemies[i].size(); ++j) {
				enemies[i][j].Draw(&program);
			}
		}

		player.Draw(&program);
	}

	// the game over screen works fine if i draw it in the main loop but here with the state swtiching consitently will use
	// the space fighter sprite sheet with the same texcoordinates for some reason

	if (state == STATE_GAME_OVER) {
		for (unsigned int i = 0; i < gameOver.size(); ++i) gameOver[i].Draw(&program);
	}
}

void ProcessEvents(SDL_Event &event, std::vector<std::vector<Entity>> &enemies, std::vector<Entity> &enemyProjectiles, Entity &player, 
	std::vector<Entity> &playerProjectiles, float elapsed, bool &done) {

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (state == STATE_GAME_LEVEL) {

		// left right movement

		if (keys[SDL_SCANCODE_LEFT] && (player.x - player.uImage.size / 2) >= -3.5) player.x -= elapsed * 2.0f;
		if (keys[SDL_SCANCODE_RIGHT] && (player.x + player.uImage.size / 2) <= 3.5) player.x += elapsed * 2.0f;

	}

	//player shooting
	//to simplify things, random enemy shoots when you do

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {

				if (state == STATE_MAIN_MENU) {
					state = STATE_GAME_LEVEL;
				}

				if (state == STATE_GAME_LEVEL) {
					for (unsigned int i = 0; i < playerProjectiles.size(); ++i) {
						if (!playerProjectiles[i].inUse) {
							playerProjectiles[i].inUse = true;
							playerProjectiles[i].x = player.x;
							playerProjectiles[i].y = player.y;
							playerProjectiles[i].velocityY = 2.0f;
							break;
						}
					}
					int row = (float)rand() / (float)RAND_MAX * enemies.size();
					int col = (float)rand() / (float)RAND_MAX * enemies[0].size();

					for (unsigned int i = 0; i < enemyProjectiles.size(); ++i) {
						if (!enemyProjectiles[i].inUse) {
							enemyProjectiles[i].inUse = true;
							enemyProjectiles[i].x = enemies[row][col].x;
							enemyProjectiles[i].y = enemies[row][col].y;
							enemyProjectiles[i].velocityY = -2.0f;
							break;
						}
					}
				}
			}
		}
	}
}

void Update(std::vector<std::vector<Entity>> &enemies, std::vector<Entity> &playerProjectiles, std::vector<Entity> &enemyProjectiles, Entity &player, const float &elapsed) {

	// fixed movement of enemies: odd and even rows move in opposite velocities until invisible first or last enemy 
	// ship on row touches border. then reverses velocity

	for (unsigned int i = 0; i < enemies.size(); ++i) {
		for (unsigned int j = 0; j < enemies[i].size(); ++j ) {
				if (enemies[i][0].x <= -3.5f ) {
					enemies[i][j].velocityX = 0.2f;
				}
				if (enemies[i].back().x >= 3.5f) {
					enemies[i][j].velocityX = -0.2f;
				}
		}
	}

	// updates all movement

	for (unsigned int i = 0; i < enemies.size(); ++i) {
		for (unsigned int j = 0; j < enemies[i].size(); ++j) {
			enemies[i][j].x += elapsed * enemies[i][j].velocityX;
		}
	}

	for (unsigned int i = 0; i < playerProjectiles.size(); ++i) {
		playerProjectiles[i].y += elapsed * playerProjectiles[i].velocityY;
	}

	for (unsigned int i = 0; i < enemyProjectiles.size(); ++i) {
		enemyProjectiles[i].y += elapsed * enemyProjectiles[i].velocityY;
	}

	//collison

	//player projectile - enemy collision

	//size of hitboxes are guessed because i'm not sure how to get them accurately so they might be outta wack

	for (unsigned int i = 0; i < playerProjectiles.size(); ++i) {
		if (playerProjectiles[i].inUse) {
			for (unsigned int j = 0; j < enemyProjectiles.size(); ++j) { //collision with enemy projectile
				if (enemyProjectiles[j].inUse) {
					if (!(playerProjectiles[i].y - 0.15 / 2 >= enemyProjectiles[j].y + 0.15 / 2 ||
						playerProjectiles[i].y + 0.15 / 2 <= enemyProjectiles[j].y - 0.15 / 2 ||
						playerProjectiles[i].x - 0.35 / 2 >= enemyProjectiles[j].x + 0.35 / 2 ||
						playerProjectiles[i].x + 0.35 / 2 <= enemyProjectiles[j].x - 0.35 / 2)) {
						playerProjectiles[i].x = 4.0f;
						playerProjectiles[i].velocityX = 0.0f;
						playerProjectiles[i].inUse = false;
						enemyProjectiles[j].x = 4.0f;
						enemyProjectiles[i].velocityX = 0.0f;
						enemyProjectiles[j].inUse = false;
					}
				}
			}

			for (unsigned int k = 0; k < enemies.size(); ++k) {
				for (unsigned int l = 0; l < enemies[k].size(); ++l) {  // collsion with enemy
					if (enemies[k][l].inUse && (l != 0) && (l != enemies[k].size() - 1)) {  //there a bug where a unshootable enemy pops up on the right of each row.
						if (!(playerProjectiles[i].y - 0.15 / 2 >= enemies[k][l].y + 0.45 / 2 ||  //i think its in here
							playerProjectiles[i].y + 0.15 / 2 <= enemies[k][l].y - 0.45 / 2 ||
							playerProjectiles[i].x - 0.35 / 2 >= enemies[k][l].x + 0.35 / 2 ||
							playerProjectiles[i].x + 0.35 / 2 <= enemies[k][l].x - 0.35 / 2)) {
							playerProjectiles[i].x = 4.0f;
							playerProjectiles[i].velocityX = 0.0f;
							playerProjectiles[i].inUse = false;
							enemies[k][l].x = 4.0f;
							enemies[k][l].velocityX = 0.0f;
							enemies[k][l].inUse = false;
						}
					}
				}
			}
		}
	}

	// enemy projectile - player collision
	for (unsigned int m = 0; m < enemyProjectiles.size(); ++m) {
		if (enemyProjectiles[m].inUse) {
			for (unsigned int n = 0; n < enemyProjectiles.size(); ++n) {
				if (!(enemyProjectiles[m].y - 0.15 / 2 >= player.y + 0.45 / 2 ||
					enemyProjectiles[m].y + 0.15 / 2 <= player.y - 0.45 / 2 ||
					enemyProjectiles[m].x - 0.35 / 2 >= player.x + 0.35 / 2 ||
					enemyProjectiles[m].x + 0.35 / 2 <= player.x - 0.35 / 2)) {
					enemyProjectiles[n].x = 4.0f;
					enemyProjectiles[n].velocityX = 0.0f;
					enemyProjectiles[n].inUse = false;
					state = STATE_GAME_OVER;

				}
			}
		}
	}




}

void Entity::Draw(ShaderProgram *program) { //assumes the entity must have a even sprite or uneven sprite

	Matrix modelMatrix;

	modelMatrix.setPosition(x, y, 0.0f);
	modelMatrix.setRotation(angle);
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();

	if (EvenOrUneven()) eImage.Draw(program);
	else uImage.Draw(program);

}

void UnevenSheetSprite::Draw(ShaderProgram* program) {

	glBindTexture(GL_TEXTURE_2D, textureID);

	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};

	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f  * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size,
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void EvenSheetSprite::Draw(ShaderProgram* program) {

	float u = (float)(((int)index) % x) / (float)x;
	float v = (float)(((int)index) / x) / (float)y;
	float spriteWidth = 1.0f / (float)x;
	float spriteHeight = 1.0f / (float)y;

	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float aspect = spriteWidth / spriteHeight;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f  * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size,
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}