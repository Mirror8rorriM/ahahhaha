#pragma once

#include "models.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <set>

namespace efd {

class Hero;

class ResourceManager {
public:
    explicit ResourceManager(std::string dataDirectory = "data");

    void loadAll();

    const std::map<std::string, LocationData>& locations() const { return locations_; }
    const std::map<std::string, EnemyData>& enemies() const { return enemies_; }
    const std::vector<CombatPhrase>& combatPhrases() const { return combatPhrases_; }
    const std::vector<DialogueEntry>& dialogues() const { return dialogues_; }

    const LocationData& location(const std::string& id) const;
    const EnemyData& enemy(const std::string& id) const;
    std::vector<DialogueEntry> dialoguesFor(const std::string& npcId, int day, const std::string& locationId) const;

    void saveGame(const std::string& path, const GameState& state, const Hero& hero) const;
    bool loadGame(const std::string& path, GameState& state, Hero& hero) const;

private:
    std::filesystem::path path(const std::string& fileName) const;
    void loadLocations();
    void loadEnemies();
    void loadCombatPhrases();
    void loadDialogues();

    std::filesystem::path dataDirectory_;
    std::map<std::string, LocationData> locations_;
    std::map<std::string, EnemyData> enemies_;
    std::vector<CombatPhrase> combatPhrases_;
    std::vector<DialogueEntry> dialogues_;
};

} // namespace efd
