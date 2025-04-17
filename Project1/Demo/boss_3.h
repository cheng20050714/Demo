#ifndef _BOSS_3_H_
#define _BOSS_3_H_

#include "boss.h"

extern Atlas atlas_boss_3; // ������ Boss �Ķ���ͼ��

class Boss3 : public Boss {
public:
    Boss3() : Boss(&atlas_boss_3) {
        position.y = 500.0f; // ���� y ����Ϊ��Ļ�߶ȵ�һ��
        position.x = 1280.0f; // ���� x ����Ϊ��Ļ�Ҳ�
    }

protected:
    void on_move(float delta) override {
        // ���ҵ����ƶ�
        position.x -= speed * delta;
        if (position.x < -100) { // ���������Ļ��࣬���õ��Ҳ�
            position.x = 1280; // ������Ļ���Ϊ 1280
        }
    }
};

#endif // !_BOSS_3_H_
