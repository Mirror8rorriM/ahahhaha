
#include "Player.h"

Player::Player(int x, int y, int cellSize)
    : Creature(x, y)  
{
  if (playerTexture.loadFromFile("player.png")) {
    playerSprite.setTexture(playerTexture);
    float texW = static_cast<float>(playerTexture.getSize().x);
    float texH = static_cast<float>(playerTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      playerSprite.setScale(static_cast<float>(cellSize) / texW,
                            static_cast<float>(cellSize) / texH);
    }
  }
  this->hp = 100;
  this->dmg = 6;
  damageTimer.restart();
}

void Player::Attack(Creature& creature) {
  if (attackClock.getElapsedTime().asSeconds() >= attack_speed) {
    creature.TakeDmg(dmg);
    attackClock.restart();
  }
}

void Player::TakeDmg(int dmg_to_take) {
  if (damageTimer.getElapsedTime().asSeconds() < damageCooldown) {
    return;
  }
  Creature::TakeDmg(dmg_to_take);
  damageTimer.restart();
}

void Player::Draw(sf::RenderWindow& window, int cellSize) {
  if (playerTexture.loadFromFile("player.png")) {
    playerSprite.setTexture(playerTexture);
    float texW = static_cast<float>(playerTexture.getSize().x);
    float texH = static_cast<float>(playerTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      playerSprite.setScale(static_cast<float>(cellSize) / texW,
                            static_cast<float>(cellSize) / texH);
    }
  }
  sf::Sprite sprite = playerSprite;
  sprite.setPosition(x_pos * cellSize, y_pos * cellSize);
  window.draw(sprite);
}

void Player::Heal(int amount) { hp = std::min(hp + amount, 100); }
