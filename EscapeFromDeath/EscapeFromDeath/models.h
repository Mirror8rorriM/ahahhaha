#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <set>

namespace efd {

struct LocationData {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> neighbors;
};

struct EnemyData {
    std::string id;
    std::string name;
    int hp = 30;
    int damage = 10;
    double timeScale = 1.0;
};

struct CombatReaction {
    std::string word;
    std::string text;
};

struct CombatPhrase {
    std::string attackText;
    std::vector<CombatReaction> reactions;
};

struct DialogueEntry {
    std::string npcId;
    int minDay = 1;
    std::string locationId;
    std::string text;
};

struct GameState {
    int day = 1;
    int actionsLeft = 5;
    std::string currentLocationId = "cell";
    int lastTrainingDay = -100;
    int lastShowerDay = -100;
    int trainingCount = 0;
    std::set<std::string> inventory;
    std::map<std::string, bool> flags;

    bool hasItem(const std::string& item) const {
        return inventory.find(item) != inventory.end();
    }

    void addItem(const std::string& item) {
        inventory.insert(item);
    }

    void removeItem(const std::string& item) {
        inventory.erase(item);
    }

    bool flag(const std::string& key) const {
        auto it = flags.find(key);
        return it != flags.end() && it->second;
    }

    void setFlag(const std::string& key, bool value = true) {
        flags[key] = value;
    }
};

} // namespace efd
