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
    // ���캯������ʼ����ʱ��
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

    // ���ö���
    void reset() {
        timer.restart();
        idx_frame = 0;
    }

    // ���ö���λ��
    void set_position(const Vector2& position) {
        this->position = position;
    }
    // ���ö�����ת�Ƕ�
    void set_rotation(double angle) {
        this->angle = angle;
    }
    // ���ö������ĵ�
    void set_center(const SDL_FPoint& center) {
        this->center = center;
    }
    // ���ö����Ƿ�ѭ��
    void set_loop(bool is_loop) {
        this->is_loop = is_loop;
    }
    // ���ö���֡���ʱ��
    void set_interval(float interval) {
        timer.set_wait_time(interval);
    }
    // ���ö�������ʱ�Ļص�����
    void set_on_finished(std::function<void()>on_finished) {
        this->on_finished = on_finished;
    }

    // ���֡��������
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

    // ��ͼ�����֡��������
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

    // ���¶���
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
    Vector2 position; // ����λ��
    double angle = 0; // ������ת�Ƕ�
    SDL_FPoint center = { 0 }; // �������ĵ�

    Timer timer; // ��ʱ��
    bool is_loop = true; // �Ƿ�ѭ��
    size_t idx_frame = 0; // ��ǰ֡����
    std::vector<Frame> frame_list; // ֡�б�
    std::function<void()> on_finished; // ��������ʱ�Ļص�����
};

#endif // !_ANIMATION_H_
