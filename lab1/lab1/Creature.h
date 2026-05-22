#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Map.h"
class Creature {
 protected:
  int hp;
  int dmg;
  std::string sprite;
  bool is_alive = true;
  float attack_speed = 0.5f;
  sf::Clock attackClock;

 public:
  float x_pos;
  float y_pos;
  Creature(int x, int y);
  virtual void TakeDmg(int dmg_to_take);
  void Deth();
  virtual void Attack(Creature* creature) = 0;
  int GetHp() const;
  int GetDmg() const;
  sf::Vector2f getPosition() const;
  void setPosition(float x, float y);
  bool IsAlive() const;
};
