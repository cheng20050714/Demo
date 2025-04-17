#ifndef _BOSS_3_H_
#define _BOSS_3_H_

#include "boss.h"

extern Atlas atlas_boss_3; // 第三个 Boss 的动画图集

class Boss3 : public Boss {
public:
    Boss3() : Boss(&atlas_boss_3) {
        position.y = 500.0f; // 设置 y 坐标为屏幕高度的一半
        position.x = 1280.0f; // 设置 x 坐标为屏幕右侧
    }

protected:
    void on_move(float delta) override {
        // 从右到左移动
        position.x -= speed * delta;
        if (position.x < -100) { // 如果超出屏幕左侧，重置到右侧
            position.x = 1280; // 假设屏幕宽度为 1280
        }
    }
};

#endif // !_BOSS_3_H_
