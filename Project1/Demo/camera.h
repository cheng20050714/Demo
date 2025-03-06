#ifndef _CAMERA_H_
#define _CAMERA_H_

#include"timer.h"
#include"vector2.h"

#include<SDL.h>

class Camera {
public:
	//�ڴ����ϻ��ƣ��������Ϊ�����ߣ�Ҳ��Ϊ�����
	Camera(SDL_Renderer* renderer) {
		this->renderer = renderer;
		timer_shaker.set_one_shot(true);
		timer_shaker.set_on_timeout([&]()
			{
				is_shaking = false;
				reset();
			});
	}

	~Camera() = default;

	const Vector2& get_position()const {
		return position;
	}
	void reset() {
		position.x = 0;
		position.y = 0;
	}

	void on_update(float delta) {
		timer_shaker.on_update(delta);
		if (is_shaking) {
			position.x = (-50 + rand() % 100) / 50.0f * shaking_strength;
			position.y = (-50 + rand() % 100) / 50.0f * shaking_strength;
		}
	}
	//strength: ��������
	//duration������ʱ��
	void shake(float strength, float duration) {
		is_shaking = true;
		shaking_strength = strength;
		timer_shaker.set_wait_time(duration);
		timer_shaker.restart();
	}

	//func����Ⱦ����
	//center������������
	//parameter:�������Դ����Ŀ�������Ƕȣ�����
	void render_texture(SDL_Texture* texture, const SDL_Rect* rect_src,
		const SDL_FRect* rect_dst,double angle,const SDL_FPoint* center)const {
		SDL_FRect rect_dst_win = *rect_dst;
		rect_dst_win.x -= position.x;
		rect_dst_win.y -= position.y;

		SDL_RenderCopyExF(renderer, texture, rect_src,
			&rect_dst_win, angle, center, SDL_RendererFlip::SDL_FLIP_NONE);
	}


private:
	Vector2 position;
	Timer timer_shaker;
	bool is_shaking = false;
	float shaking_strength = 0;
	SDL_Renderer* renderer = nullptr;
};



#endif // !_CAMERA_H_
