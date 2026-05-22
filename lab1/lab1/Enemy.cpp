#include "Enemy.h"

#include <cmath>
#include <typeinfo>

Enemy::Enemy(int x, int y, int id, bool move_x) : Creature(x, y) {
  this->id = id;
  this->move_x = move_x;
  // дефолтные значени€; потомки могут переопредел€ть
  this->hp = 10;
  this->dmg = 1;
  attackTimer.restart();  // стартируем таймер
}

void Enemy::Ai(Map& map, Player& player) {
  // передаЄм игрока Ч Move проверит столкновение
  Move(map, player);
}

void Enemy::Move(Map& map, Player& player) {
  sf::Vector2f nextPos = {x_pos, y_pos};
  if (move_x)
    nextPos.x += move_pos;
  else
    nextPos.y += move_pos;

  int nx = static_cast<int>(std::floor(nextPos.x));
  int ny = static_cast<int>(std::floor(nextPos.y));
  int px = static_cast<int>(std::floor(player.x_pos));
  int py = static_cast<int>(std::floor(player.y_pos));

  // если целева€ клетка Ч игрок, атакуем только если истЄк таймер
  if (nx == px && ny == py) {
    if (attackTimer.getElapsedTime().asSeconds() >= attack_speed) {
      Attack(player);
      attackTimer.restart();
    }
    return;
  }

  if (!map.isWall(nextPos.x, nextPos.y) &&
      !map.isEnemyAt(nextPos.x, nextPos.y)) {
    x_pos = nextPos.x;
    y_pos = nextPos.y;
    if (id >= 0 && id < static_cast<int>(map.enemies.size()))
      map.enemies[id] = {x_pos, y_pos};
  } else {
    move_pos = -move_pos;
  }
}

void Enemy::Attack(Creature& creature) {
  try {
    Player& player = dynamic_cast<Player&>(creature);
    float dx = player.x_pos - x_pos;
    float dy = player.y_pos - y_pos;
    float distance = std::hypot(dx, dy);

    if (distance <= static_cast<float>(attack_range)) {
      if (attackTimer.getElapsedTime().asSeconds() >= attack_speed) {
        player.TakeDmg(dmg);
        attackTimer.restart();
      }
    }
  } catch (const std::bad_cast&) {
    return;
  }
}

// Ѕазова€ отрисовка (виртуальна€)
void Enemy::Draw(sf::RenderWindow& window, int cellSize) const {
  sf::RectangleShape shape(sf::Vector2f(static_cast<float>(cellSize - 1),
                                        static_cast<float>(cellSize - 1)));
  shape.setFillColor(sf::Color::Red);
  shape.setPosition(x_pos * cellSize, y_pos * cellSize);
  window.draw(shape);
}

// Dog
Dog::Dog(int x, int y, int id, bool move_x, int cellSize)
    : Enemy(x, y, id, move_x) {
  if (enemyTexture.loadFromFile("dog.png")) {
    enemySprite.setTexture(enemyTexture);
    float texW = static_cast<float>(enemyTexture.getSize().x);
    float texH = static_cast<float>(enemyTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      enemySprite.setScale(static_cast<float>(cellSize) / texW,
                           static_cast<float>(cellSize) / texH);
    }
  }
  if (deadEnemyTexture.loadFromFile("deadDog.png")) {
    deadEnemySprite.setTexture(deadEnemyTexture);
    float texW = static_cast<float>(deadEnemyTexture.getSize().x);
    float texH = static_cast<float>(deadEnemyTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      deadEnemySprite.setScale(static_cast<float>(cellSize) / texW,
                               static_cast<float>(cellSize) / texH);
    }
  }
  this->hp = 30;
  this->dmg = 10;
  attackTimer.restart();
}

void Dog::Folowing(Player& player, Map& map) {
  sf::Vector2f playerPos = {player.x_pos, player.y_pos};
  sf::Vector2f nextPos = {x_pos, y_pos};
  float diffX = playerPos.x - x_pos;
  float diffY = playerPos.y - y_pos;
  if (std::abs(diffX) > std::abs(diffY))
    nextPos.x += (diffX > 0) ? 1 : -1;
  else
    nextPos.y += (diffY > 0) ? 1 : -1;

  int nx = static_cast<int>(std::floor(nextPos.x));
  int ny = static_cast<int>(std::floor(nextPos.y));
  int px = static_cast<int>(std::floor(player.x_pos));
  int py = static_cast<int>(std::floor(player.y_pos));

  if (nx == px && ny == py) {
    if (attackTimer.getElapsedTime().asSeconds() >= attack_speed) {
      Attack(player);
      attackTimer.restart();
    }
    return;
  }

  if (!map.isWall(nextPos.x, nextPos.y) &&
      !map.isEnemyAt(nextPos.x, nextPos.y)) {
    x_pos = nextPos.x;
    y_pos = nextPos.y;
    if (id >= 0 && id < static_cast<int>(map.enemies.size()))
      map.enemies[id] = {x_pos, y_pos};
  }
}

void Dog::Ai(Map& map, Player& player) {
  float diffX = player.x_pos - x_pos;
  float diffY = player.y_pos - y_pos;
  if (std::abs(diffX) <= follow_range || std::abs(diffY) <= follow_range) {
    Folowing(player, map);
  } else {
    Move(map, player);
  }
}

void Dog::Draw(sf::RenderWindow& window, int cellSize) const {
  sf::Sprite sprite;
  if (is_alive)
    sprite = enemySprite;
  else
    sprite = deadEnemySprite;
  sprite.setPosition(x_pos * cellSize, y_pos * cellSize);
  window.draw(sprite);
}

// Sceleton
Sceleton::Sceleton(int x, int y, int id, bool move_x, int cellSize)
    : Enemy(x, y, id, move_x) {
  if (enemyTexture.loadFromFile("sceleton.png")) {
    enemySprite.setTexture(enemyTexture);
    float texW = static_cast<float>(enemyTexture.getSize().x);
    float texH = static_cast<float>(enemyTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      enemySprite.setScale(static_cast<float>(cellSize) / texW,
                           static_cast<float>(cellSize) / texH);
    }
  }
  if (deadEnemyTexture.loadFromFile("deadSceleton.png")) {
    deadEnemySprite.setTexture(deadEnemyTexture);
    float texW = static_cast<float>(deadEnemyTexture.getSize().x);
    float texH = static_cast<float>(deadEnemyTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      deadEnemySprite.setScale(static_cast<float>(cellSize) / texW,
                               static_cast<float>(cellSize) / texH);
    }
  }
  this->hp = 20;
  this->dmg = 15;
  attackTimer.restart();
}

void Sceleton::Ai(Map& map, Player& player) { Move(map, player); }

void Sceleton::Draw(sf::RenderWindow& window, int cellSize) const {
  sf::Sprite sprite;
  if (is_alive)
    sprite = enemySprite;
  else
    sprite = deadEnemySprite;
  sprite.setPosition(x_pos * cellSize, y_pos * cellSize);
  window.draw(sprite);
}
