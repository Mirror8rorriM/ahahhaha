
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Enemy.h"
#include "Item.h"
#include "Map.h"
#include "Player.h"

class Game {
 private:
  Map map;
  Player player;
  std::vector<std::unique_ptr<Enemy>> enemies;
  std::vector<std::unique_ptr<Item>> chests;
  std::vector<std::unique_ptr<Item>> kyes;
  sf::Clock shootClock;
  sf::Clock teleportTimer;
  int shootTurn;
  int cellSize;
  int allkyes = 0;
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
  void PlayerMovement(sf::Event& event, int& shootDistance);
  void PlayerMove(sf::Event& event);
  int PlayerShoot(sf::Event& event);
  void PlayerTp(sf::Event& event);
  void DrawGrid(sf::RenderWindow& window, const sf::View& view);
  void DrawEnemy(sf::RenderWindow& window);
  void DrawChests(sf::RenderWindow& window);
  void DrawKeys(sf::RenderWindow& window);
  void drawUI(sf::RenderWindow& window, sf::View view, int hp);
  void isPlayerOnChest();
  void isPlayerOnKey();
  void DrawShoot(sf::RenderWindow& window, int shootDistance);
  void DrawGameOver(sf::RenderWindow& window);

 public:
  Game();
  
  void BigText(std::string str,sf::Color color);
  void Run();
  void loadEnemiesFromFile(const std::string& filename);
};
