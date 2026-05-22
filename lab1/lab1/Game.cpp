
#include "Game.h"

#include <fstream>
#include <iostream>
enum class ActiveType {
  DogX = 2,
  DogY,
  SceletonX,
  SceletonY,
  Chest_first,
  Chest_Second,
  Final_Chest,
  key
};

Game::Game() : map("map.txt", 30), player(1, 1, cellSize) {
  cellSize = static_cast<int>(map.getCellSize());
  if (chestTexture.loadFromFile("chest.png")) {
    chestSprite.setTexture(chestTexture);
    float texW = static_cast<float>(chestTexture.getSize().x);
    float texH = static_cast<float>(chestTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      chestSprite.setScale(static_cast<float>(cellSize) / texW,
                           static_cast<float>(cellSize) / texH);
    }
  }
  if (keyTexture.loadFromFile("key.png")) {
    keySprite.setTexture(keyTexture);
    float texW = static_cast<float>(keyTexture.getSize().x);
    float texH = static_cast<float>(keyTexture.getSize().y);
    if (texW > 0.0f && texH > 0.0f) {
      keySprite.setScale(static_cast<float>(cellSize) / texW,
                         static_cast<float>(cellSize) / texH);
    }
  }
}
void Game::PlayerMove(sf::Event& event) {
  sf::Vector2f nextPos = player.getPosition();

  if (event.key.code == sf::Keyboard::W) {
    nextPos.y -= 1;
    player.turn = static_cast<int>(Turn::up);
  } else if (event.key.code == sf::Keyboard::S) {
    nextPos.y += 1;
    player.turn = static_cast<int>(Turn::down);
  } else if (event.key.code == sf::Keyboard::A) {
    nextPos.x -= 1;
    player.turn = static_cast<int>(Turn::left);
  } else if (event.key.code == sf::Keyboard::D) {
    nextPos.x += 1;
    player.turn = static_cast<int>(Turn::right);
  }

  if (map.isEnemyAt(nextPos.x, nextPos.y)) {
    for (const auto& enemy : enemies) {
      if (enemy->getPosition() == nextPos) {
        player.Attack(*enemy);
        return;
      }
    }
  } else if (!map.isWall(nextPos.x, nextPos.y)) {
    player.setPosition(nextPos.x, nextPos.y);
    return;
  }
}
int Game::PlayerShoot(sf::Event& event) {
  if (shootClock.getElapsedTime().asSeconds() >= player.GetPistolCooldown() &&
      player.CanShoot()) {
    if (event.key.code == sf::Keyboard::Up) {
      shootTurn = static_cast<int>(Turn::up);
      shootClock.restart();
      for (int i = 1; i <= player.GetPistolRange(); ++i) {
        for (const auto& enemy : enemies) {
          if (enemy->getPosition() ==
              sf::Vector2f(player.x_pos, player.y_pos - i)) {
            enemy->TakeDmg(player.GetPistolDmg());
            return i;
          }
        }
        if (map.isWall(player.x_pos, player.y_pos - i)) return i;
      }

    } else if (event.key.code == sf::Keyboard::Down) {
      shootTurn = static_cast<int>(Turn::down);
      shootClock.restart();
      for (int i = 1; i <= player.GetPistolRange(); ++i) {
        for (const auto& enemy : enemies) {
          if (enemy->getPosition() ==
              sf::Vector2f(player.x_pos, player.y_pos + i)) {
            enemy->TakeDmg(player.GetPistolDmg());
            return i;
          }
        }
        if (map.isWall(player.x_pos, player.y_pos + i)) return i;
      }
    } else if (event.key.code == sf::Keyboard::Left) {
      shootTurn = static_cast<int>(Turn::left);
      shootClock.restart();
      for (int i = 1; i <= player.GetPistolRange(); ++i) {
        for (const auto& enemy : enemies) {
          if (enemy->getPosition() ==
              sf::Vector2f(player.x_pos - i, player.y_pos)) {
            enemy->TakeDmg(player.GetPistolDmg());
            return i;
          }
        }
        if (map.isWall(player.x_pos - i, player.y_pos)) return i;
      }
    } else if (event.key.code == sf::Keyboard::Right) {
      shootTurn = static_cast<int>(Turn::right);
      shootClock.restart();
      for (int i = 1; i <= player.GetPistolRange(); ++i) {
        for (const auto& enemy : enemies) {
          if (enemy->getPosition() ==
              sf::Vector2f(player.x_pos + i, player.y_pos)) {
            enemy->TakeDmg(player.GetPistolDmg());
            return i;
          }
        }
        if (map.isWall(player.x_pos + i, player.y_pos)) return i;
      }
    }
  }
  return player.GetPistolRange();
}
void Game::PlayerTp(sf::Event& event) {
  if (event.key.code == sf::Keyboard::LShift && player.CanTeleport() &&
      teleportTimer.getElapsedTime().asSeconds() >=
          player.GetTeleportCooldown()) {
    sf::Vector2f nextPos = player.getPosition();
    if (player.turn == static_cast<int>(Turn::up)) {
      nextPos.y -= player.GetTeleportRange();
    } else if (player.turn == static_cast<int>(Turn::down)) {
      nextPos.y += player.GetTeleportRange();
    } else if (player.turn == static_cast<int>(Turn::left)) {
      nextPos.x -= player.GetTeleportRange();
    } else if (player.turn == static_cast<int>(Turn::right)) {
      nextPos.x += player.GetTeleportRange();
    }
    if (!map.isWall(nextPos.x, nextPos.y) &&
        !map.isEnemyAt(nextPos.x, nextPos.y)) {
      player.setPosition(nextPos.x, nextPos.y);
      teleportTimer.restart();
    }
  }
}
void Game::PlayerMovement(sf::Event& event, int& shootDistance) {
  if (event.type == sf::Event::KeyPressed) {
    PlayerMove(event);
    shootDistance = PlayerShoot(event);
    PlayerTp(event);
  }
}

void Game::Run() {
  sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight),
                          "Infinite World & Center Player");
  window.setFramerateLimit(60);

  loadEnemiesFromFile("map.txt");
  sf::View view(sf::FloatRect(0, 0, windowWidth, windowHeight));  // camera

  sf::Clock enemyClock;
  sf::Clock attackClock;
  const float moveDelay = 0.5f;
  const float attackDelay = 1.0f;
  int shootDistance = 0;
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed ||
          (event.type == sf::Event::KeyPressed &&
           event.key.code == sf::Keyboard::Escape))
        window.close();

      PlayerMovement(event, shootDistance);
    }
    if (enemyClock.getElapsedTime().asSeconds() >= moveDelay) {
      for (const auto& enemy : enemies) {
        if (enemy->IsAlive()) {
          enemy->Ai(map, player);
        }
      }
      enemyClock.restart();
    }
    if (attackClock.getElapsedTime().asSeconds() >= attackDelay) {
      for (const auto& enemy : enemies) {
        if (enemy->IsAlive()) {
          enemy->Attack(player);
        }
      }
      attackClock.restart();
    }
    if (win) {
      window.close();
      BigText("You Win", sf::Color::Green);
    }
    if (!player.IsAlive()) {
      window.close();
      BigText("Game Over", sf::Color::Red);
    }
    isPlayerOnChest();
    isPlayerOnKey();
    view.setCenter(player.getPosition().x * cellSize + cellSize / 2.f,
                   player.getPosition().y * cellSize + cellSize / 2.f);
    window.setView(view);

    window.clear();

    // 1. Сначала рисуем фон и карту (они в самом низу)
    DrawGrid(window, view);
    map.draw(window);  // Карта нарисует пол и стены

    // 2. Затем рисуем объекты, которые стоят на полу
    DrawChests(window);
    DrawKeys(window);   // Ключи
    DrawEnemy(window);  // Враги
    player.Draw(window, cellSize);
    if (shootClock.getElapsedTime().asSeconds() <= timeShootDrowing) {
      DrawShoot(window, shootDistance);
    }

    // 3. В самом конце — интерфейс (всегда поверх всего)
    drawUI(window, view, player.GetHp());

    window.display();
  }
}

void Game::DrawEnemy(sf::RenderWindow& window) {
  for (const auto& e : enemies) {
    if (e) e->Draw(window, cellSize);  // каждый враг рисует себя
  }
}

void Game::loadEnemiesFromFile(const std::string& filename) {
  std::ifstream file(filename);
  std::string line;
  int row = 0;
  while (std::getline(file, line)) {
    int col = 0;
    for (char c : line) {
      int code = c - '0';
      // в файле: строка = row (y), символ = col (x)
      // Map::loadFromFile добавляет рамку +1, поэтому смещаем
      int spawnX = col + 1;
      int spawnY = row + 1;

      if (code == static_cast<int>(ActiveType::DogX) ||
          code == static_cast<int>(ActiveType::DogY)) {
        bool moveX = (code == static_cast<int>(ActiveType::DogX));
        enemies.push_back(std::make_unique<Dog>(
            spawnX, spawnY, static_cast<int>(enemies.size()), moveX, cellSize));
        map.enemies.push_back(sf::Vector2f(spawnX, spawnY));
      } else if (code == static_cast<int>(ActiveType::SceletonX) ||
                 code == static_cast<int>(ActiveType::SceletonY)) {
        bool moveX = (code == static_cast<int>(ActiveType::SceletonX));
        enemies.push_back(std::make_unique<Sceleton>(
            spawnX, spawnY, static_cast<int>(enemies.size()), moveX, cellSize));
        map.enemies.push_back(sf::Vector2f(spawnX, spawnY));
      } else if (code == static_cast<int>(ActiveType::Chest_first)) {
        chests.push_back(std::make_unique<Item>(
            spawnX, spawnY, static_cast<int>(ActiveType::Chest_first)));
      } else if (code == static_cast<int>(ActiveType::Chest_Second)) {
        chests.push_back(std::make_unique<Item>(
            spawnX, spawnY, static_cast<int>(ActiveType::Chest_Second)));
      } else if (code == static_cast<int>(ActiveType::Final_Chest)) {
        chests.push_back(std::make_unique<Item>(
            spawnX, spawnY, static_cast<int>(ActiveType::Final_Chest)));
      } else if (code == static_cast<int>(ActiveType::key)) {
        kyes.push_back(std::make_unique<Item>(
            spawnX, spawnY, static_cast<int>(ActiveType::key)));
        allkyes++;
      }
      col++;
    }
    row++;
  }
  file.close();
}
void Game::DrawGrid(sf::RenderWindow& window, const sf::View& view) {
  sf::Vector2f viewCenter = view.getCenter();
  // Границы видимой области в пикселях
  int startX =
      static_cast<int>(viewCenter.x - windowWidth / 2) / cellSize * cellSize -
      cellSize;
  int startY =
      static_cast<int>(viewCenter.y - windowHeight / 2) / cellSize * cellSize -
      cellSize;
  int endX = startX + windowWidth + cellSize;
  int endY = startY + windowHeight + cellSize;

  for (int i = startX; i < endX; i += cellSize) {
    for (int j = startY; j < endY; j += cellSize) {
      sf::RectangleShape cell(
          sf::Vector2f(static_cast<float>(cellSize - gridWidth),
                       static_cast<float>(cellSize - gridWidth)));
      cell.setPosition(static_cast<float>(i), static_cast<float>(j));
      cell.setFillColor(sf::Color(30, 30, 30));
      window.draw(cell);
    }
  }
}
void Game::DrawChests(sf::RenderWindow& window) {
  sf::Sprite sprite = chestSprite;

  for (const auto& c : chests) {
    if (c && c->active) {
      sf::Vector2f pos = c->getPosition();  // клеточные координаты
      sprite.setPosition(pos.x * cellSize, pos.y * cellSize);
      window.draw(sprite);
    }
  }
}
void Game::DrawKeys(sf::RenderWindow& window) {
  sf::Sprite sprite = keySprite;

  for (const auto& k : kyes) {
    if (k && k->active) {
      sf::Vector2f pos = k->getPosition();  // клеточные координаты
      sprite.setPosition(pos.x * cellSize, pos.y * cellSize);
      window.draw(sprite);
    }
  }
}
void Game::drawUI(sf::RenderWindow& window, sf::View view, int hp) {
  window.setView(window.getDefaultView());
  font.loadFromFile("arial.ttf");
  sf::Text hpText;
  hpText.setFont(font);
  hpText.setString("HP: " + std::to_string(hp));
  hpText.setCharacterSize(24);
  hpText.setFillColor(sf::Color::White);
  hpText.setStyle(sf::Text::Bold);

  float x = window.getSize().x - hpText.getGlobalBounds().width - 20;
  float y = 20;
  hpText.setPosition(x, y);

  window.draw(hpText);
  sf::Text keysText;
  keysText.setFont(font);
  keysText.setString("Keys: " + std::to_string(player.GetKeysCollected()) +
                     "/" + std::to_string(allkyes));
  keysText.setCharacterSize(24);
  keysText.setFillColor(sf::Color::White);
  keysText.setStyle(sf::Text::Bold);

  x = 20;
  y = 20;
  keysText.setPosition(x, y);
  window.draw(keysText);
  if (player.CanShoot()) {
    sf::Text shootText;
    shootText.setFont(font);
    if (shootClock.getElapsedTime().asSeconds() >= player.GetPistolCooldown())
      shootText.setString("Shoot:Ready");
    else {
      shootText.setString("Shoot:Cooldown " +
                          std::to_string(static_cast<int>(
                              player.GetPistolCooldown() -
                              shootClock.getElapsedTime().asSeconds())) +
                          "s");
    }
    shootText.setCharacterSize(24);
    shootText.setFillColor(sf::Color::White);
    shootText.setStyle(sf::Text::Bold);

    x = 20;
    y = window.getSize().y - shootText.getGlobalBounds().height - 20;
    shootText.setPosition(x, y);
    window.draw(shootText);
  }
  if (player.CanTeleport()) {
    sf::Text teleportText;
    teleportText.setFont(font);
    if (teleportTimer.getElapsedTime().asSeconds() >=
        player.GetTeleportCooldown())
      teleportText.setString("Teleport:Ready");
    else {
      teleportText.setString("Teleport:Cooldown " +
                             std::to_string(static_cast<int>(
                                 player.GetTeleportCooldown() -
                                 teleportTimer.getElapsedTime().asSeconds())) +
                             "s");
    }
    teleportText.setCharacterSize(24);
    teleportText.setFillColor(sf::Color::White);
    teleportText.setStyle(sf::Text::Bold);
    x = window.getSize().x - teleportText.getGlobalBounds().width - 20;
    y = window.getSize().y - teleportText.getGlobalBounds().height - 20;
    teleportText.setPosition(x, y);
    window.draw(teleportText);
  }
  window.setView(view);
}
void Game::isPlayerOnChest() {
  for (const auto& item : chests) {
    if (item->active && item->getPosition() == player.getPosition()) {
      if (item->GetType() == static_cast<int>(ActiveType::Chest_first)) {
        player.EnableShooting();
        item->active = false;

      } else if (item->GetType() ==
                 static_cast<int>(ActiveType::Chest_Second)) {
        player.EnableTeleportation();
        item->active = false;

      } else if (item->GetType() == static_cast<int>(ActiveType::Final_Chest) &&
                 allkyes <= player.GetKeysCollected()) {
        win = true;
        item->active = false;
      }
    }
  }
}
void Game::isPlayerOnKey() {
  for (const auto& item : kyes) {
    if (item->active && item->getPosition() == player.getPosition()) {
      player.CollectKey();

      item->active = false;
    }
  }
}

void Game::DrawShoot(sf::RenderWindow& window, int shootDistance) {
  if (shootDistance > 0) {
    sf::RectangleShape shootShape(sf::Vector2f(static_cast<float>(cellSize),
                                               static_cast<float>(cellSize)));
    shootShape.setFillColor(sf::Color::White);
    float offsetX = 0, offsetY = 0;
    if (shootTurn == static_cast<int>(Turn::up)) {
      offsetY = -shootDistance * cellSize;
    } else if (shootTurn == static_cast<int>(Turn::down)) {
      offsetY = shootDistance * cellSize;
    } else if (shootTurn == static_cast<int>(Turn::left)) {
      offsetX = -shootDistance * cellSize;
    } else if (shootTurn == static_cast<int>(Turn::right)) {
      offsetX = shootDistance * cellSize;
    }
    shootShape.setPosition(player.x_pos * cellSize + offsetX,
                           player.y_pos * cellSize + offsetY);
    window.draw(shootShape);
  }
}
void Game::BigText(std::string str, sf::Color color) {
  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Game Over Screen");
  window.setFramerateLimit(60);

  sf::Font font;

  font.loadFromFile("arial.ttf");

  sf::Text text;
  text.setFont(font);
  text.setString(str);
  text.setCharacterSize(80);
  text.setFillColor(color);
  text.setStyle(sf::Text::Bold);
  /*text.setColor(sf::Color::Red);*/
  sf::FloatRect textRect = text.getLocalBounds();
  text.setOrigin(textRect.left + textRect.width / 2.0f,
                 textRect.top + textRect.height / 2.0f);
  text.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
  window.clear(sf::Color(20, 20, 20));
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close();

      if (event.type == sf::Event::KeyPressed &&
          event.key.code == sf::Keyboard::Escape)
        window.close();
    }

    window.clear(sf::Color(20, 20, 20));  // Темный фон

    // Рисуем сначала тень, потом текст

    window.draw(text);

    window.display();
  }
}
