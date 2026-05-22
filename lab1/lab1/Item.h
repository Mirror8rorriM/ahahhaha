#pragma once
#include <SFML/Graphics.hpp>
class Item {
 private:
  int type;

 public:
  int x_pos;
  int y_pos;
  Item(int x, int y, int type);
  bool active = true;
  sf::Vector2f getPosition() const { return sf::Vector2f(x_pos, y_pos); }
  int GetType() const { return type; }
  int GetType() { return type; }
};
