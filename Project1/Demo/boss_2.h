#ifndef _BOSS_2_H_
#define _BOSS_2_H_

#include "boss.h"

extern Atlas atlas_boss_2; // 第二个 Boss 的动画图集

class Boss2 : public Boss {
public:
    Boss2() : Boss(&atlas_boss_2) {
        position.y = 200.0f; // 设置 y 坐标为屏幕高度的一半
    }

protected:
    void on_move(float delta) override {
        // 从左到右移动
        position.x += speed * delta;
        if (position.x > 1280) { // 如果超出屏幕右侧，重置到左侧
            position.x = -100; // 假设 Boss 的宽度为 100
        }
    }
};

#endif // !_BOSS_2_H_
