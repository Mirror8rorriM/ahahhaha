
#include "Creature.h"

void Creature::TakeDmg(int dmg_to_take) {
  hp -= dmg_to_take;
  if (hp <= 0) {
    Deth();
  }
}

Creature::Creature(int x, int y) {
  this->hp = 10;
  this->dmg = 1;
  x_pos = static_cast<float>(x);
  y_pos = static_cast<float>(y);
}

sf::Vector2f Creature::getPosition() const { return {x_pos, y_pos}; }

void Creature::setPosition(float x, float y) {
  x_pos = x;
  y_pos = y;
}
bool Creature::IsAlive() const { return is_alive; }
void Creature::Deth() { is_alive = false; }
int Creature::GetHp() const { return hp; }
int Creature::GetDmg() const { return dmg; }
