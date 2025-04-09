#include "Players.h"

void Enemy::damagePlayer(Player& player) {
	time_t currentTime = clock();
	// Check if at least 1 second (CLOCKS_PER_SEC) has passed since last damage
	if (this->Alive) {
		if (((float)(currentTime - lastDamageTimeEnemy) / CLOCKS_PER_SEC >= 1.0f) && player.checkDistance(*this) < 5) {
			player.health -= dmg;
			lastDamageTimeEnemy = currentTime;
			player.timeSinceCombat = 0;
		}
	}
}

int Player::checkhealth() {
	return health;
}

//Not used. For future features 
int Player::checkshield() {
	return shield;
}
void Player::levelup() {
	health = (level+ 4) * 10;
	shield = level;
	dmg = dmg + (level/2);
}
void Player::addXP() {
	if (level <= 9) {
		levelProgress++;
		if (levelProgress >= level * 1.15) {
			level++;
			levelProgress = 0;
			levelup();
		}
	}
}
float Player::getHealthPercentage() {
	return (float)health / (float)100;
}

bool Player::damageEnemy(Enemy& enemy, Enemy& activeEnemy) {
	time_t currentTime = clock();
	// Check if at least 1 second (CLOCKS_PER_SEC) has passed since last damage
	if (((float)(currentTime - lastDamageTimePlayer) / CLOCKS_PER_SEC >= 0.5f) && checkDistance(activeEnemy) < 10) {
		activeEnemy.health -= dmg;
		lastDamageTimePlayer = currentTime;
		if (activeEnemy.health <= 0) {
			activeEnemy.Alive = false;
			return true;
		}
	}
	return false;
}

float Player::checkDistance(Enemy& enemy) {
	vec3 center = enemy.object->toWorld * vec3(0, 0, 0);
	mat4 mvInv = Inverse(camera->modelview);
	vec3 camPos = mvInv * vec3(0, 0, 0);
	return length(camPos - center);
}

void Player::regen() {
	time_t currentTime = clock();
	if ((health < (level + 4) * 10) && ((float)(currentTime - timeSinceCombat) / CLOCKS_PER_SEC >= 3.0f)) {
		health = health + 1;
		timeSinceCombat = currentTime;
	}
}

//moves the alive enemy twoards the player
void Enemy::moveEnemy(Player& player) {
	if (!this->Alive) return; // Don't move if the enemy is dead
	// Get player and enemy positions
	mat4 mvInv = Inverse(player.camera->modelview);
	vec3 playerPos = mvInv * vec3(0, 0, 0); // Player position in world space
	vec3 enemyPos = object->toWorld * vec3(0, 0, 0); // Enemy position
	float distance = length(playerPos - enemyPos);
	// Calculate direction vector toward the player
	vec3 direction = normalize(playerPos - enemyPos);
	// Update enemy position
	if (distance > 2) {
		mat4 move = Translate(direction.x * moveSpeed, direction.y * moveSpeed, direction.z * moveSpeed);
		object->toWorld = move * object->toWorld;
	}
}

int Enemy::enemyCounter = 0;