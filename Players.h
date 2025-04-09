#ifndef PLAYERS_H
#define PLAYERS_H

#include <ctime>
#include "Camera.h"
#include "Mesh.h"

class Enemy;

class Player {
public:
	int level = 1;
	int levelProgress = 0;
	int health = 50;
	//Not used. For future features 
	int shield = 0;
	int dmg = 1;
	//Not used. For future features 
	int range = 0;
	time_t lastDamageTimePlayer = 0;
	Camera* camera = NULL;
	time_t timeSinceCombat = 100;
	int checkhealth();
	//Not used. For future features 
	int checkshield();
	void levelup();
	void addXP();
	float getHealthPercentage();
	void regen();
	bool damageEnemy(Enemy& enemy, Enemy& activeEnemy);//return true if enemy is still alive. False if enemy is dead.
	float checkDistance(Enemy& enemy);
	Player(Camera* c) : camera(c) {}
};

class Enemy {
public:
	static int enemyCounter;
	int enemynum;
	int health;
	int dmg;
	float moveSpeed = 0.05f;
	time_t lastDamageTimeEnemy = 0;
	bool Alive = true;
	bool Boss;
	Mesh* object = NULL;

	Enemy(bool isAlive, bool isBoss = false) : Alive(isAlive), Boss(isBoss), enemynum(++enemyCounter) {
		// Assign health and damage AFTER enemynum is set
		health = (isBoss ? 23 : 10) * (enemynum/1.5);
		dmg = (isBoss ? 50 : 5 * (enemynum / 1.3));
		moveSpeed = (isBoss ? .115f : .085f);
	}
	void damagePlayer(Player& player);
	void moveEnemy(Player& player);
	
}; 

#endif
