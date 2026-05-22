#include "Map.h"

#include <cctype>
#include <iostream>
#include <sstream>
bool Map::isEnemyAt(float x, float y) const {
  int gx = static_cast<int>(std::floor(x));
  int gy = static_cast<int>(std::floor(y));

  for (const sf::Vector2f& e : enemies) {
    if (static_cast<int>(std::floor(e.x)) == gx &&
        static_cast<int>(std::floor(e.y)) == gy) {
      return true;
    }
  }
  return false;
}
Map::Map(std::string filename, int size) {
  cellSize = size;
  if (wallTexture.loadFromFile("wall.png")) {
    wallSprite.setTexture(wallTexture);
    float texW = static_cast<float>(wallTexture.getSize().x);
    float texH = static_cast<float>(wallTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      wallSprite.setScale(static_cast<float>(cellSize) / texW,
                          static_cast<float>(cellSize) / texH);
    }
  }
  loadFromFile(filename);
}
void Map::loadFromFile(std::string filename) {
  std::ifstream file(filename);
  std::vector<std::vector<int>> tempGrid;
  std::string line;

  while (std::getline(file, line)) {
    std::vector<int> row;
    std::stringstream ss(line);

    int number;
    while (ss >> number) {
      row.push_back(number);
    }

    if (!row.empty()) tempGrid.push_back(row);
  }

  file.close();

  if (tempGrid.empty()) return;

  int mapHeight = tempGrid.size();
  int mapWidth = tempGrid[0].size();
  for (std::vector<int> i : tempGrid) {
    int mapWidth = std::max(static_cast<int>(i.size()), mapWidth);
  }

  grid.assign(mapHeight + 2, std::vector<int>(mapWidth + 2, 1));

  for (int i = 0; i < mapHeight; ++i) {
    for (int j = 0; j < mapWidth; ++j) {
      grid[i + 1][j + 1] = tempGrid[i][j];
    }
  }
}
bool Map::isWall(float x, float y) const {
  int gridX = static_cast<int>(x);
  int gridY = static_cast<int>(y);

  if (gridY >= 0 && gridY < grid.size() && gridX >= 0 &&
      gridX < grid[0].size()) {
    return grid[gridY][gridX] == 1;
  }
  return true;
}

void Map::draw(sf::RenderWindow& window) {
  for (int row = 0; row < static_cast<int>(grid.size()); ++row) {
    for (int col = 0; col < static_cast<int>(grid[row].size()); ++col) {
      if (grid[row][col] == 1) {
        float px = static_cast<float>(col * cellSize);
        float py = static_cast<float>(row * cellSize);
        wallSprite.setPosition(px, py);
        window.draw(wallSprite);
      }
    }
  }
}
