#ifndef _BOSS_1_H_
#define _BOSS_1_H_

#include "boss.h"

extern Atlas atlas_boss_1; // ��һ�� Boss �Ķ���ͼ��

class Boss1 : public Boss {
public:
    Boss1() : Boss(&atlas_boss_1) {
        speed = 15.0f; // ���ٶȴ�Ĭ�ϵ� 10.0f ��ߵ� 15.0f
        position.x = 600.0f; 

    }

protected:
    void on_move(float delta) override {
        // ֮�����ƶ��߼�
        position.x += speed * delta * (std::sin(time_elapsed * 2.0f) > 0 ? 1 : -1); // ���Ұڶ�
        position.y += speed * delta * 0.5f; // ���������ƶ�
    }
};

#endif // !_BOSS_1_H_
