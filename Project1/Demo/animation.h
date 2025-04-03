#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "atlas.h"
#include "camera.h"
#include "vector2.h"
#include "timer.h"

#include <vector>
#include <functional>

class Animation {
public:
    // 构造函数，初始化定时器
    Animation() {
        timer.set_one_shot(false);
        timer.set_on_timeout(
            [&]()
            {
                idx_frame++;
                if (idx_frame >= frame_list.size()) {
                    idx_frame = is_loop ? 0 : frame_list.size() - 1;
                    if (!is_loop && on_finished) {
                        on_finished();
                    }
                }
            }
        );
    }
    ~Animation() = default;

    // 重置动画
    void reset() {
        timer.restart();
        idx_frame = 0;
    }

    // 设置动画位置
    void set_position(const Vector2& position) {
        this->position = position;
    }
    // 设置动画旋转角度
    void set_rotation(double angle) {
        this->angle = angle;
    }
    // 设置动画中心点
    void set_center(const SDL_FPoint& center) {
        this->center = center;
    }
    // 设置动画是否循环
    void set_loop(bool is_loop) {
        this->is_loop = is_loop;
    }
    // 设置动画帧间隔时间
    void set_interval(float interval) {
        timer.set_wait_time(interval);
    }
    // 设置动画结束时的回调函数
    void set_on_finished(std::function<void()>on_finished) {
        this->on_finished = on_finished;
    }

    // 添加帧到动画中
    void add_frame(SDL_Texture* texture, int num_h) {
        int width, height;
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
        int width_frame = width / height;

        for (int i = 0; i < num_h; i++) {
            SDL_Rect rect_src;
            rect_src.x = i * width_frame, rect_src.y = 0;
            rect_src.w = width_frame, rect_src.h = height;
            
            frame_list.emplace_back(texture, rect_src);
        }
    }

    // 从图集添加帧到动画中
    void add_frame(Atlas* atlas) {
        for (int i = 0; i < atlas->get_size(); i++) {
            SDL_Texture* texture = atlas->get_texture(i);

            int width, height;
            SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
            
            SDL_Rect rect_src;
            rect_src.x = 0, rect_src.y = 0;
            rect_src.w = width, rect_src.h = height;

            frame_list.emplace_back(texture, rect_src);
        }
    }

    // 更新动画
    void on_update(float delta) {
        timer.on_update(delta);
    }

    void on_render(const Camera& camera) const {
        if (frame_list.empty()) {
            return; // or handle the error appropriately
        }
        const Frame& frame = frame_list[idx_frame];
        const Vector2& pos_camera = camera.get_position();

        SDL_FRect rect_dst;
        rect_dst.x = position.x - frame.rect_src.w / 2;
        rect_dst.y = position.y - frame.rect_src.h / 2;
        rect_dst.w = (float)frame.rect_src.w;
        rect_dst.h = (float)frame.rect_src.h;

        camera.render_texture(frame.texture, &frame.rect_src, &rect_dst, angle, &center);
    }

private:
    struct Frame {
        SDL_Rect rect_src;
        SDL_Texture* texture = nullptr;

        Frame() = default;
        Frame(SDL_Texture* texture, const SDL_Rect& rect_src)
            : texture(texture), rect_src(rect_src) { }
        ~Frame() = default;
    };

private:
    Vector2 position; // 动画位置
    double angle = 0; // 动画旋转角度
    SDL_FPoint center = { 0 }; // 动画中心点

    Timer timer; // 定时器
    bool is_loop = true; // 是否循环
    size_t idx_frame = 0; // 当前帧索引
    std::vector<Frame> frame_list; // 帧列表
    std::function<void()> on_finished; // 动画结束时的回调函数
};

#endif // !_ANIMATION_H_
