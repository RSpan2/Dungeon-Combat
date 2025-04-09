// Dungeon_combat.cpp
// A Dungeon Combat game where the user fights 9 enemies and a boss.
// Copyright (c) 2025 Rocco Spannaus, Samir Ali, all rights reserved. Commercial use requires license.

#include <glad.h>
#include <glfw3.h>
#include <unordered_map>
#include "Camera.h"
#include "Cubemap.h"
#include "Draw.h"
#include "GLXtras.h"
#include "Mesh.h"
#include "Sprite.h"
#include "Widgets.h"
#include "Text.h"
#include "Players.h"

//Window
GLFWwindow* w;

//sliders
vec2	points[] = { {-.5f, .5f} };
vec2* selectedPoint = NULL;
Slider* selectedSlider = NULL;
vec3	wht(1, 1, 1), blk(0, 0, 0);
float sens = .1f;
Slider sensitivity = Slider(vec2(.5f), vec2(-.0f, 0.f), 0.001f, 1, sens, false, 12, wht);

// window, camera, lights, options
int		winWidth = 2000, winHeight = 2000;
Camera	camera(0, 0, winWidth, winHeight, vec3(0, 0, 0), vec3(0, 0, -20));
vec3	lights[] = { {1, 2, 0}, {-3, 3, 2} };

//Sprites and buttons
enum		GameState { GS_Unstarted = 0, Playing, Pause, Settings, Dead, Won, NStates };
string		stateNames[] = { "Unstarted", "Playing", "Pause", "Settings", "Dead", "Won"};
GameState	gameState = GS_Unstarted;
Sprite		background, start, quit, settings, back, HealthBar_Empty, HealthBar, restart, Damage;
Sprite* buttons[] = { &start, nullptr, &quit, &settings, &back, &restart };

// meshes and cubemap
Mesh	ground, slime, goblin, tree, walls[4], roof;
CubeMap	sky;

// interaction
unordered_map<int, time_t> keyDowntime;
mat4	cameraInitial;

//Sprinting
bool shiftPressed = false;

//variables used to display hitmarker
bool damage = false;
bool resetDamage = false;
time_t Time = clock();
time_t timeSinceDamage;

//Mouse Camera Control
float lastx = 0;
bool lastx_init = false;

//Enemy and player declarations
Enemy E0(true);
Enemy E1(false, false);
Enemy E2(false, false);
Enemy E3(false, false);
Enemy E4(false, false);
Enemy E5(false, false);
Enemy E6(false, false);
Enemy E7(false, false);
Enemy E8(false, false);
Enemy E9(false, false);
Enemy E10(false, true);
bool bossAlive = false;
Enemy* activeEnemy = &E0;
Enemy *enemies[] = { &E0, &E1,  &E2, &E3,  &E4, &E5,  &E6, &E7, &E8, &E9, &E10 };
Player P1(&camera);

//Checks distance from obj
float checkDistance(Mesh *object) {
	vec3 center = object->toWorld * vec3(0, 0, 0);
	mat4 mvInv = Inverse(camera.modelview);
	vec3 camPos = mvInv * vec3(0, 0, 0);
	return length(camPos - center);
}

//restarts the game from beginning
void Restart() {
	P1.level = 1;
	P1.levelProgress = 0;
	P1.health = 50;
	P1.shield = 0;
	P1.dmg = 1;
	P1.range = 0;
	bossAlive = false;
	camera.SetModelview(camera.modelview);
	for (int i = 0; i < (sizeof(enemies) / sizeof(enemies[0])); i++) {
		activeEnemy = enemies[i];
		activeEnemy->Alive = false;
		activeEnemy->health = (activeEnemy->Boss ? 23 : 10) * (activeEnemy->enemynum / 1.5);
	}
	E0.Alive = true;
	activeEnemy->Alive = false;
	activeEnemy = &E0;
	gameState = Playing;
}

// Display
void Display() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (gameState == GS_Unstarted) {
		background.SetFrame(gameState);
		background.Display();
		(buttons[gameState])->Display();
		quit.Display();
		glDisable(GL_DEPTH_TEST);
	}
	else if (gameState == Playing) {
		time_t Time = clock();  // Update Time each frame
		//hitmarker display
		if (damage && !resetDamage) {
			timeSinceDamage = Time;
			resetDamage = true;
		}
		if (resetDamage) {
			Damage.Display();// Keep displaying the damage sprite
			// If .5 second has passed, reset the damage state
			if ((float)(Time - timeSinceDamage) / CLOCKS_PER_SEC >= 0.5f) {
				resetDamage = false;
				damage = false;
			}
		}
		// set lights
		int s = UseMeshShader();
		int nLights = sizeof(lights) / sizeof(vec3);
		SetUniform(s, "nLights", nLights);
		SetUniform3v(s, "lights", nLights, (float*)lights, camera.modelview);
		// meshes and cubemap
		sky.Display(camera);
		ground.Display(camera);
		for (int i = 0; i < 4; i++) {
			walls[i].Display(camera, 0);
		}
		roof.Display(camera, 0);
		for (Enemy* e : enemies) {
			if (e->Alive) {
				e->object->Display(camera);
			}
		}
		checkDistance(activeEnemy->object);
		activeEnemy->damagePlayer(P1);
		P1.regen();
		activeEnemy->moveEnemy(P1);
		// display health bar
		float healthPercent = P1.getHealthPercentage();
		float f = .35f * healthPercent;// width of health bar
		vec2 pos = HealthBar_Empty.ptTransform * vec2(-1, 0);// left edge of empty health bar
		HealthBar.ptTransform = Translate(pos.x, pos.y, 0) * Translate(f, 0, 0) * Scale(f, HealthBar.scale.y, 1);
		// above, reading right-to-left:
		// scale health bar horizontally by health
		// shift to right by health so left edge now at origin
		// shift to left edge of empty health bar
		HealthBar.Display();
		HealthBar_Empty.Display();
		// display gamestate, camera/object distance
	}
	else if (gameState == Pause) {
		background.SetFrame(1);
		background.Display();
		(buttons[gameState])->Display();
		settings.Display();
		glDisable(GL_DEPTH_TEST);
	}
	else if (gameState == Settings) {
		background.SetFrame(1);
		background.Display();
		back.Display();
		glDisable(GL_DEPTH_TEST);
		sensitivity.Draw();
		//popup.Display();
		UseDrawShader();
		if (selectedPoint)
			Disk(*selectedPoint, 40, blk);
	}
	else if (gameState == Dead) {
		glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		background.SetFrame(3);
		background.Display();
		quit.SetPosition(vec2(-0.5f, -0.5f));
		restart.SetPosition(vec2(0.5f, -0.5f));
		quit.Display();
		restart.Display();
		glDisable(GL_DEPTH_TEST);
	}
	else if (gameState == Won) {
		glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		background.SetFrame(4);
		background.Display();
		quit.SetPosition(vec2(0.0f, -0.5f));
		quit.Display();
		glDisable(GL_DEPTH_TEST);
	}
	glFlush();
}

//Changes gamestate to playing
void startgame() {
	glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	gameState = Playing;
	lastx_init = false;
}
// Interactions
//Movement with WASD and Sprinting
void SetCamera(int key) {
	if (gameState == Playing) {
		vec3 move(0, 0, 0);  // Movement vector
		float speed = shiftPressed ? 0.15f : 0.1f;
		// Determine movement direction
		if (key == GLFW_KEY_A) move.x = speed;
		if (key == GLFW_KEY_D) move.x = -speed;
		if (key == GLFW_KEY_W) move.z = speed;
		if (key == GLFW_KEY_S) move.z = -speed;
		// Apply movement
		mat4 m = Translate(move);
		// Collision Detection
		vec3 currentCamera = Inverse(camera.modelview) * vec3(0, 0, 0);
		mat4 movedModelview = m * camera.modelview;
		vec3 proposedCamera = Inverse(movedModelview) * vec3(0, 0, 0);
		// Define wall
		vec3 sq[4] = { {1, -1, 0}, {1, 1, 0}, {-1, 1, 0}, {-1, -1, 0} };
		bool segmentIntersects = false;
		for (int w = 0; w < 4; w++){ //loop through walls
			vec3 p[4];
			for (int i = 0; i < 4; i++) {
				p[i] = walls[w].toWorld * sq[i];
			}
			// Check collision
			QuadInfo q(p[0], p[1], p[2], p[3]);
			float alpha = 0;
			bool lineIntersects = q.IntersectWithLine(currentCamera, proposedCamera, &alpha);
			if (lineIntersects && alpha >= 0 && alpha <= 1) {
				segmentIntersects = true;
			}
		}
		if (!segmentIntersects) {
			camera.SetModelview(movedModelview);
		}
	}
}

//Checks Keyboard input to reset the camera, interact with objs, enter pause screen
void Keyboard(int key, bool press, bool shift, bool control) {
	if (gameState == Playing) {
		shiftPressed = shift;
		if (press) {
			if (key == 'R')
				camera.SetModelview(cameraInitial);
			keyDowntime[key] = clock();
			SetCamera(key);
		}
		if (press) {
			if (key == GLFW_KEY_ESCAPE) {
				gameState = Pause;
				quit.SetPosition(vec2(0.0f, -0.7f));
				settings.SetPosition(vec2(0.0f, -0.2f));
				glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		if (press) {
			if (key == 'K') {
				P1.dmg = P1.dmg + 20;
			}
		}
		if (press) {
			if (key == 'L') {
				P1.health = P1.health + 10;
			}
		}
	}
	else if (gameState == Pause) {
		shiftPressed = shift;
		if (press) {
			if (key == GLFW_KEY_ESCAPE) {
				startgame();
			}
		}
	}
}

//Camera Control and slider interaction
void MouseMove(float x, float y, bool leftDown, bool rightDown) {
	if (gameState == Playing && lastx_init) {
		float xrot = (float)(x - lastx) * sens;
		mat4 rotateY = RotateY(xrot);
		camera.SetModelview(rotateY * camera.modelview);
	}
	if (gameState == Playing && !lastx_init) {
		lastx_init = true;
	}
	lastx = x;
	if (gameState == Settings) {
		if (leftDown) {
			vec2 ndc = NDCfromScreen(x, y);
			if (selectedSlider) {
				selectedSlider->Drag(ndc);
				sens = selectedSlider->Value();
			}	
			else if (selectedPoint)
				*selectedPoint = ndc;
		}
	}
 }

//Checks if the mouse is over a button when pressed
void MouseButton(float x, float y, bool left, bool down) {
	if (gameState == GS_Unstarted) {
		glEnable(GL_DEPTH_TEST);
		if (down && start.Hit(x, y))
			startgame();
		else if (down && quit.Hit(x, y)) {
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
	}
	else if (gameState == Pause) {
		glEnable(GL_DEPTH_TEST);
		if (down && quit.Hit(x, y)) {
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
		if (down && settings.Hit(x, y)) {
			gameState = Settings;
		}
	}
	else if (gameState == Settings) {
		selectedSlider = NULL;
		if (left && down) {
			//selectedSlider = popup.Down(x, y);
			vec2 ndc = NDCfromScreen(x, y);
			if (sensitivity.Hit(ndc)) {
				selectedSlider = &sensitivity;
			}
		}
		if (down && back.Hit(x, y)) {
			gameState = Pause;
		}
	}
	else if (gameState == Playing) {
		if (checkDistance(activeEnemy->object) < 10) {
			P1.damageEnemy(E0, *activeEnemy);
			damage = true;
			if (!activeEnemy->Alive) {
				activeEnemy->Alive = false;
				int nextEnemyIndex = activeEnemy->enemynum + 1;
				P1.addXP();
				if (nextEnemyIndex < (sizeof(enemies) / sizeof(enemies[0])) && enemies[nextEnemyIndex] != nullptr) {
					activeEnemy = enemies[activeEnemy->enemynum + 1];
					std::srand(std::time(0));
					// Generate a random number between 1 and 100 (inclusive)
					int x = (std::rand() % 25) + 1;
					int z = (std::rand() % 25) + 1;
					if (activeEnemy->enemynum == 10) {
						activeEnemy->object = &slime;
						bossAlive = true;
					}
					else if (activeEnemy->enemynum % 2 == 0) {
						activeEnemy->object = &tree;
						
						tree.toWorld = Translate(x, 0, z) * Scale(1);
					}
					else {
						activeEnemy->object = &goblin;
						goblin.toWorld = Translate(x, 0, z) * Scale(1);
					}
					activeEnemy->Alive = true;
				}
			}
		}
	}
	else if (gameState == Dead) {
		glEnable(GL_DEPTH_TEST);
		if (down && quit.Hit(x, y)) {
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
		if (down && restart.Hit(x, y)) {
			Restart();
		}
	}
	else if (gameState == Won) {
		glEnable(GL_DEPTH_TEST);
		if (down && quit.Hit(x, y)) {
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
	}
}

//Continuous movement as key is held
void CheckKey() {
	if (gameState == Playing) {
		for (int k : { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D }) {
			time_t t = keyDowntime[k];
			if (KeyDown(k) && t > 0 && (float)(clock() - t) / CLOCKS_PER_SEC > .00001f)
				SetCamera(k);
		}
	}
}

// Application
//Initializes Sprites
void InitSprite(Sprite& s, string name, vec2 scale, vec2 position) {
	s.Initialize(name, -.5f, true);
	s.SetScale(scale);
	s.SetPosition(position);
}

//Resizes the camera when window is resized
void Resize(int width, int height) {
	camera.Resize(width, height);
	glViewport(0, 0, width, height);
}

//Prints to the console
const char *usage = R"(
	Start:			Starts game
	Quit :			Quits Game
	Settings:		Enters Settings
	Back:			Goes to previous page
	Mouse:			rotate ccw about local origin
	WASD:			Move around
	ESC:			Enter and exit pause screen
	Shift:			Sprint
	Mouse Button:	Damage Enemy When within 10 units
	CHEATS:
	L:				Adds 20 damage to player
	K:				Adds ten health	to player			
	R:				reset view
)";

//Checks to see if player or boos is dead
void isDead() {
	if (P1.checkhealth() <= 0) {
		gameState = Dead;
	}
	if (bossAlive && !E10.Alive) {
		gameState = Won;
	}
}

string dir = "C:/Users/Jules/Code/GG-Projects/2025/15-DungeonCombat/";
int main(int ac, char **av) {
	E0.object = &tree;
	for (int i = 0; i < (sizeof(enemies) / sizeof(enemies[0])); i++) {
		enemies[i]->enemynum = i;
	}
	w = InitGLFW(100, 100, winWidth, winHeight, "Dungeon Combat");
	//Initializes Backgrounds
	vector<string> backgrounds = { "C:\\Code\\CPSC4270\\Assets\\Images\\Start_Screen.png", "C:\\Code\\CPSC4270\\Assets\\Images\\Pause_background.png","C:\\Code\\CPSC4270\\Assets\\Images\\Light_blue.png", "C:\\Code\\CPSC4270\\Assets\\Images\\death_Screen.png", "C:\\Code\\CPSC4270\\Assets\\Images\\win_Screen.png" };
	background.Initialize(backgrounds, "", 0, false);
	background.compensateAspectRatio = true;
	background.autoAnimate = false;
	InitSprite(start, "C:\\Code\\CPSC4270\\Assets\\Images\\Start.png", vec2(.1f, .15f)* 1.5f, vec2(-.3f, -.6f));
	InitSprite(quit, "C:\\Code\\CPSC4270\\Assets\\Images\\quit.png", vec2(.1f, .15f) * 1.5f, vec2(.3f, -.6f));
	InitSprite(settings, "C:\\Code\\CPSC4270\\Assets\\Images\\settings.png", vec2(.2f, .2f) * 1.5f, vec2(.25f, -.6f));
	InitSprite(back, "C:\\Code\\CPSC4270\\Assets\\Images\\back_button.png", vec2(.1f, .2f) * 1.5f, vec2(.0f, -.6f));
	InitSprite(restart, "C:\\Code\\CPSC4270\\Assets\\Images\\Restart.png", vec2(.1f, .15f) * 1.5f, vec2(.0f, -.6f));
	InitSprite(HealthBar_Empty, "C:\\Code\\CPSC4270\\Assets\\Images\\HealthBar\\HealthBar_Empty.png", vec2(.35f, .05f), vec2(.625f, .9f));
	InitSprite(HealthBar, "C:\\Code\\CPSC4270\\Assets\\Images\\HealthBar\\HealthBar.png", vec2(.35f, .05f), vec2(.625f, .9f));
	InitSprite(Damage, "C:\\Code\\CPSC4270\\Assets\\Images\\Damage.png", vec2(.5f, .5f), vec2(-.75f, 0.75f));
	//Reads in objects
	slime.Read("C:\\Code\\CPSC4270\\Assets\\Models\\slime.obj");
	goblin.Read("C:\\Code\\CPSC4270\\Assets\\Models\\goblin.obj");
	tree.Read("C:\\Code\\CPSC4270\\Assets\\Models\\tree.obj");
	ground.Read("C:\\Code\\CPSC4270\\Assets\\Models\\Square.obj");
	for (int i = 0; i < 4; i++) {
		walls[i].Read("C:\\Code\\CPSC4270\\Assets\\Models\\Square.obj", "C:\\Code\\CPSC4270\\Assets\\Images\\BrickTexture.tga");
	}
	roof.Read("C:\\Code\\CPSC4270\\Assets\\Models\\Square.obj", "C:\\Code\\CPSC4270\\Assets\\Images\\BrickTexture.tga");
	sky.Read("C:/Code/CPSC4270/Assets/Images/SkyCubemap.png");
	// transform objects into scene
	ground.toWorld = Translate(0, -.8f, 0)*RotateX(-90)*Scale(50);
	walls[0].toWorld = Translate(0, 5.0f, -50) * Scale(50, 8, 1);
	walls[1].toWorld = Translate(50, 5.0f, 0) * RotateY(-90) * Scale(50, 8, 1);
	walls[2].toWorld = Translate(0, 5.0f, 50) * Scale(50, 8, 1);
	walls[3].toWorld = Translate(-50, 5.0f, 0) * RotateY(-90) * Scale(50, 8, 1);
	roof.toWorld = Translate(0, 10, 0) * RotateX(-90) * Scale(50);
	cameraInitial = camera.modelview;
	// callbacks
	RegisterMouseButton(MouseButton);
	RegisterResize(Resize);
	RegisterKeyboard(Keyboard);
	printf("Usage:%s", usage);
	RegisterMouseMove(MouseMove);
	lastx = (float) winWidth / 2;
	// event loop
	while (!glfwWindowShouldClose(w)) {
		isDead();
		CheckKey();
		Display();
		glfwSwapBuffers(w);
		glfwPollEvents();
	}
}
