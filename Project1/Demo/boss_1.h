#ifndef _BOSS_1_H_
#define _BOSS_1_H_

#include "boss.h"

extern Atlas atlas_boss_1; // 第一个 Boss 的动画图集

class Boss1 : public Boss {
public:
    Boss1() : Boss(&atlas_boss_1) {
        speed = 15.0f; // 将速度从默认的 10.0f 提高到 15.0f
        position.x = 600.0f; 

    }

protected:
    void on_move(float delta) override {
        // 之字形移动逻辑
        position.x += speed * delta * (std::sin(time_elapsed * 2.0f) > 0 ? 1 : -1); // 左右摆动
        position.y += speed * delta * 0.5f; // 缓慢向下移动
    }
};

#endif // !_BOSS_1_H_
