#ifndef _BOSS_H_
#define _BOSS_H_

#include "camera.h"
#include "vector2.h"
#include "animation.h"
#include <SDL_mixer.h>
#include <iostream>

// 外部资源
extern Mix_Chunk* sound_explosion; // 爆炸音效
extern Mix_Music* boss_bgm; // boss出现音效
extern Atlas atlas_boss_explosion; // 通用爆炸动画图集

class Boss {
public:
    // 构造函数接受动画图集
    Boss(Atlas* run_atlas) {
        // 初始化通用爆炸动画
        animation_explosive.set_loop(false);
        animation_explosive.set_interval(0.08f);
        animation_explosive.add_frame(&atlas_boss_explosion);
        animation_explosive.set_on_finished([&]() {
            is_valid = false; // 动画播放完后，Boss 无效
            });

        // 初始化运行动画
        animation_run.set_loop(true);
        animation_run.set_interval(0.1f);
        animation_run.add_frame(run_atlas);

        // 设置初始位置
        position.x = 0.0f;
        position.y = 0.0f;

        // 设置移动速度
        speed = 20.0f;

        // 设置初始血量
        hp = 20;

        // 初始化时间变量
        time_elapsed = 0.0f;


    }

    virtual ~Boss() = default;

    // 获取 Boss 的位置
    const Vector2& get_position() const {
        return position;
    }

    // 更新 Boss 的状态
    void on_update(float delta) {
        if (is_valid) {
            // 更新时间
            time_elapsed += delta;

            // 调用派生类的移动逻辑
            on_move(delta);

            // 限制 Boss 的移动范围（防止超出屏幕）
            if (position.x < 0) position.x = 0;
            if (position.x > 1280) position.x = 1280; // 假设屏幕宽度为 1280
            if (position.y > 720) position.y = 720;  // 假设屏幕高度为 720
        }

        // 更新动画
        animation_current = (is_alive ? &animation_run : &animation_explosive);
        animation_current->set_position(position);
        animation_current->on_update(delta);
    }

    // 渲染 Boss
    void on_render(const Camera& camera) const {
        animation_current->on_render(camera);
    }

    // 处理被击中
    void on_hurt() {
        if (is_alive) {
            hp--; // 每次被击中，血量减少 1
            if (hp <= 0) {
                is_alive = false; // 血量为 0 时死亡
                Mix_PlayChannel(-1, sound_explosion, 0); // 播放爆炸音效
            }
            if (hp == 20) { // 仅在 hp 等于 20 且音乐未播放时播放
                if (Mix_PlayMusic(boss_bgm, -1) == -1) { // 播放背景音乐
                    std::cerr << "Error: Failed to play boss background music! " << Mix_GetError() << std::endl;
                }
            }
        }
    }

    // 检查是否存活
    bool check_alive() const {
        return is_alive;
    }

    // 检查是否可以移除
    bool can_remove() const {
        return !is_valid;
    }

protected:
    virtual void on_move(float delta) = 0; // 抽象移动逻辑，由派生类实现

    Vector2 position; // Boss 的位置
    float speed; // 移动速度
    int hp; // 血量
    float time_elapsed; // 存活时间，用于计算移动轨迹

    bool is_alive = true; // 是否存活
    bool is_valid = true; // 是否有效

    Animation animation_run; // 运行动画
    Animation animation_explosive; // 爆炸动画
    Animation* animation_current = nullptr; // 当前动画
};

#endif // !_BOSS_H_

