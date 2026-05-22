#include "EntityFactory.h"

std::unique_ptr<Enemy> EntityFactory::CreateEnemy(int mapCode, int x, int y,
                                                  int id, int cellSize) const {
  if (mapCode == static_cast<int>(MapEntityType::DogX) ||
      mapCode == static_cast<int>(MapEntityType::DogY)) {
    bool moveX = mapCode == static_cast<int>(MapEntityType::DogX);
    return std::make_unique<Dog>(x, y, id, moveX, cellSize);
  }

  if (mapCode == static_cast<int>(MapEntityType::SceletonX) ||
      mapCode == static_cast<int>(MapEntityType::SceletonY)) {
    bool moveX = mapCode == static_cast<int>(MapEntityType::SceletonX);
    return std::make_unique<Sceleton>(x, y, id, moveX, cellSize);
  }

  return nullptr;
}

std::unique_ptr<Item> EntityFactory::CreateItem(int mapCode, int x,
                                                int y) const {
  if (mapCode == static_cast<int>(MapEntityType::FirstChest) ||
      mapCode == static_cast<int>(MapEntityType::SecondChest) ||
      mapCode == static_cast<int>(MapEntityType::FinalChest) ||
      mapCode == static_cast<int>(MapEntityType::Key) ||
      mapCode == static_cast<int>(MapEntityType::Heal) ||
      mapCode == static_cast<int>(MapEntityType::Trap)) {
    return std::make_unique<Item>(x, y, mapCode);
  }

  return nullptr;
}
