#pragma once

#include <memory>

#include "Enemy.h"
#include "Item.h"

enum class MapEntityType {
  DogX = 2,
  DogY,
  SceletonX,
  SceletonY,
  FirstChest,
  SecondChest,
  FinalChest,
  Key,
  Heal,
  Trap
};

class EntityFactory {
 public:
  std::unique_ptr<Enemy> CreateEnemy(int mapCode, int x, int y, int id,
                                     int cellSize) const;
  std::unique_ptr<Item> CreateItem(int mapCode, int x, int y) const;
};
