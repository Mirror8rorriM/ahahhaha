#include "enemy.h"

namespace efd {

Enemy::Enemy(const EnemyData& data)
    : Creatures(data.name, data.hp, data.damage), timeScale_(data.timeScale) {}

} // namespace efd
