#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <vector>

class Map {
 private:
  std::vector<std::vector<int>> grid;
  int cellSize;
  sf::Sprite wallSprite;
  sf::Texture wallTexture;
  sf::RectangleShape floorShape;
  
 public:
  std::vector<sf::Vector2f> enemies;
  Map(std::string filename, int size);
  void loadFromFile(std::string filename);
  bool isWall(float x, float y) const;
  void draw(sf::RenderWindow& window);  // актуальная сигнатура
  bool isEnemyAt(float x, float y) const;
  float getCellSize() const {
    return static_cast<float>(cellSize);
  }  // inline определение
};
