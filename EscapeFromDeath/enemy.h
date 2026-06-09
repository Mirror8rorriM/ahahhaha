#pragma once

#include "creatures.h"
#include "models.h"

namespace efd {

class Enemy : public Creatures {
public:
    Enemy() = default;
    explicit Enemy(const EnemyData& data);

    double timeScale() const { return timeScale_; }

private:
    double timeScale_ = 1.0;
};

} // namespace efd
