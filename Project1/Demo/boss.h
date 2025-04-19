#ifndef _BOSS_H_
#define _BOSS_H_

#include "camera.h"
#include "vector2.h"
#include "animation.h"
#include <SDL_mixer.h>
#include <iostream>

// �ⲿ��Դ
extern Mix_Chunk* sound_explosion; // ��ը��Ч
extern Mix_Music* boss_bgm; // boss������Ч
extern Atlas atlas_boss_explosion; // ͨ�ñ�ը����ͼ��

class Boss {
public:
    // ���캯�����ܶ���ͼ��
    Boss(Atlas* run_atlas) {
        // ��ʼ��ͨ�ñ�ը����
        animation_explosive.set_loop(false);
        animation_explosive.set_interval(0.08f);
        animation_explosive.add_frame(&atlas_boss_explosion);
        animation_explosive.set_on_finished([&]() {
            is_valid = false; // �����������Boss ��Ч
            });

        // ��ʼ�����ж���
        animation_run.set_loop(true);
        animation_run.set_interval(0.1f);
        animation_run.add_frame(run_atlas);

        // ���ó�ʼλ��
        position.x = 0.0f;
        position.y = 0.0f;

        // �����ƶ��ٶ�
        speed = 20.0f;

        // ���ó�ʼѪ��
        hp = 20;

        // ��ʼ��ʱ�����
        time_elapsed = 0.0f;


    }

    virtual ~Boss() = default;

    // ��ȡ Boss ��λ��
    const Vector2& get_position() const {
        return position;
    }

    // ���� Boss ��״̬
    void on_update(float delta) {
        if (is_valid) {
            // ����ʱ��
            time_elapsed += delta;

            // ������������ƶ��߼�
            on_move(delta);

            // ���� Boss ���ƶ���Χ����ֹ������Ļ��
            if (position.x < 0) position.x = 0;
            if (position.x > 1280) position.x = 1280; // ������Ļ���Ϊ 1280
            if (position.y > 720) position.y = 720;  // ������Ļ�߶�Ϊ 720
        }

        // ���¶���
        animation_current = (is_alive ? &animation_run : &animation_explosive);
        animation_current->set_position(position);
        animation_current->on_update(delta);
    }

    // ��Ⱦ Boss
    void on_render(const Camera& camera) const {
        animation_current->on_render(camera);
    }

    // ��������
    void on_hurt() {
        if (is_alive) {
            hp--; // ÿ�α����У�Ѫ������ 1
            if (hp <= 0) {
                is_alive = false; // Ѫ��Ϊ 0 ʱ����
                Mix_PlayChannel(-1, sound_explosion, 0); // ���ű�ը��Ч
            }
            if (hp == 20) { // ���� hp ���� 20 ������δ����ʱ����
                if (Mix_PlayMusic(boss_bgm, -1) == -1) { // ���ű�������
                    std::cerr << "Error: Failed to play boss background music! " << Mix_GetError() << std::endl;
                }
            }
        }
    }

    // ����Ƿ���
    bool check_alive() const {
        return is_alive;
    }

    // ����Ƿ�����Ƴ�
    bool can_remove() const {
        return !is_valid;
    }

protected:
    virtual void on_move(float delta) = 0; // �����ƶ��߼�����������ʵ��

    Vector2 position; // Boss ��λ��
    float speed; // �ƶ��ٶ�
    int hp; // Ѫ��
    float time_elapsed; // ���ʱ�䣬���ڼ����ƶ��켣

    bool is_alive = true; // �Ƿ���
    bool is_valid = true; // �Ƿ���Ч

    Animation animation_run; // ���ж���
    Animation animation_explosive; // ��ը����
    Animation* animation_current = nullptr; // ��ǰ����
};

#endif // !_BOSS_H_

