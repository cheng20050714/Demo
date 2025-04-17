#ifndef _BOSS_2_H_
#define _BOSS_2_H_

#include "boss.h"

extern Atlas atlas_boss_2; // �ڶ��� Boss �Ķ���ͼ��

class Boss2 : public Boss {
public:
    Boss2() : Boss(&atlas_boss_2) {
        position.y = 200.0f; // ���� y ����Ϊ��Ļ�߶ȵ�һ��
    }

protected:
    void on_move(float delta) override {
        // �������ƶ�
        position.x += speed * delta;
        if (position.x > 1280) { // ���������Ļ�Ҳ࣬���õ����
            position.x = -100; // ���� Boss �Ŀ��Ϊ 100
        }
    }
};

#endif // !_BOSS_2_H_
