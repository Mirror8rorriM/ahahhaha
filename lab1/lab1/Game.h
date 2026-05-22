
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Enemy.h"
#include "EntityFactory.h"
#include "Item.h"
#include "Map.h"
#include "Player.h"

class Game {
 private:
  Map map;
  Player player;
  EntityFactory entityFactory;
  std::vector<std::unique_ptr<Enemy>> enemies;
  std::vector<std::unique_ptr<Item>> chests;
  std::vector<std::unique_ptr<Item>> items;
  sf::Clock shootClock;
  sf::Clock teleportTimer;
  int shootTurn;
  int cellSize;
  int allkyes = 0;
  bool inTrap = false;
  const int trapPresses = 10;
  int trapPressesLeft = trapPresses;
  const int gridWidth = 1;
  const int windowWidth = 800;
  const int windowHeight = 600;
  bool win = false;
  sf::Font font;
  float timeShootDrowing = 0.2f;
  sf::Sprite chestSprite;
  sf::Texture chestTexture;
  sf::Sprite keySprite;
  sf::Texture keyTexture;
  sf::Sprite healSprite;
  sf::Texture healTexture;
  sf::Sprite trapSprite;
  sf::Texture trapTexture;
  void PlayerMovement(sf::Event& event, int& shootDistance);
  void PlayerMove(sf::Event& event);
  int PlayerShoot(sf::Event& event);
  void PlayerTp(sf::Event& event);
  void DrawGrid(sf::RenderWindow& window, const sf::View& view);
  void DrawEnemy(sf::RenderWindow& window);
  void DrawChests(sf::RenderWindow& window);
  void DrawItems(sf::RenderWindow& window);
  void drawUI(sf::RenderWindow& window, sf::View view, int hp);
  void isPlayerOnChest();
  void isPlayerOnItem();
  void DrawShoot(sf::RenderWindow& window, int shootDistance);
  void DrawGameOver(sf::RenderWindow& window);

 public:
  Game();

  void BigText(std::string str, sf::Color color);
  void Run();
  void loadEnemiesFromFile(const std::string& filename);
};
