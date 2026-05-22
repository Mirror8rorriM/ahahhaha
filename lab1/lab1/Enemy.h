
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#include "Creature.h"
#include "Map.h"
#include "Player.h"

class Enemy : public Creature {
 protected:
  std::string type;
  bool move_x;
  int id;
  int move_pos = 1;
  sf::Sprite enemySprite;
  sf::Texture enemyTexture;
  sf::Sprite deadEnemySprite;
  sf::Texture deadEnemyTexture;
  const float attack_speed = 0.5f;
  const int attackRange = 1;
  virtual int getAttackRange() const;
  sf::Clock attackTimer;

 public:
  Enemy(int x, int y, int id, bool move_x);
  virtual void Ai(Map& map, Player& player);
  void Move(Map& map, Player& player);
  void Attack(Creature& creature) override;

  virtual void Draw(sf::RenderWindow& window, int cellSize) const;
  virtual ~Enemy() = default;
};

class Dog : public Enemy {
 private:
  const int follow_range = 5;
  const int attack_range = 1;

 public:
  Dog(int x, int y, int id, bool move_x, int cellSize);
  void Folowing(Player& player, Map& map);
  void Ai(Map& map, Player& player) override;
  void Draw(sf::RenderWindow& window, int cellSize) const override;
};

class Sceleton : public Enemy {
  const int attackRange = 2;
  int getAttackRange() const;

 public:
  Sceleton(int x, int y, int id, bool move_x, int cellSize);
  void Ai(Map& map, Player& player) override;
  void Draw(sf::RenderWindow& window, int cellSize) const override;
};
