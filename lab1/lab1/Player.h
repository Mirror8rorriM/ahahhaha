
#pragma once
#include <SFML/System.hpp>

#include "Creature.h"
#include "Map.h"
enum class Turn { down, up, left, right };
class Player : public Creature {
 private:
  float attack_speed = 0.3f;
  sf::Clock damageTimer;
  float damageCooldown = 1.0f;
  const int pistol_dmg = 15;
  const int pistol_range = 5;
  const float pistol_cooldown = 5.0f;
  bool canShoot = false;
  bool canTP = false;
  const int teleport_range = 3;
  const float teleport_cooldown = 5.0f;
  int keys_collected = 0;
  sf::Sprite playerSprite;
  sf::Texture playerTexture;

 public:
  Player(int x, int y, int cellSize);
  int turn = static_cast<int>(Turn::down);
  void Attack(Creature& creature) override;
  void Draw(sf::RenderWindow& window, int cellSize);
  void Heal(int amount);
  void TakeDmg(int dmg_to_take) override;
  void EnableShooting() { canShoot = true; }
  void EnableTeleportation() { canTP = true; }
  bool CanShoot() const { return canShoot; }
  bool CanTeleport() const { return canTP; }
  int GetPistolDmg() const { return pistol_dmg; }
  int GetPistolRange() const { return pistol_range; }
  float GetPistolCooldown() const { return pistol_cooldown; }
  int GetTeleportRange() const { return teleport_range; }
  float GetTeleportCooldown() const { return teleport_cooldown; }
  int GetKeysCollected() const { return keys_collected; }
  void CollectKey() { keys_collected++; }
};
