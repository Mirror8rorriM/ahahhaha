#pragma once

#include <algorithm>
#include <string>

namespace efd {

class Creatures {
public:
    Creatures() = default;
    Creatures(std::string name, int hp, int damage);
    virtual ~Creatures() = default;

    const std::string& name() const { return name_; }
    int hp() const { return hp_; }
    int maxHp() const { return maxHp_; }
    int damage() const { return damage_; }
    bool isAlive() const { return hp_ > 0; }

    void receiveDamage(int amount);
    void healFull();

protected:
    std::string name_ = "Безымянный";
    int hp_ = 1;
    int maxHp_ = 1;
    int damage_ = 1;
};

} // namespace efd
