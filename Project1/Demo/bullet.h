#ifndef _BULLET_H_
#define _BULLET_H_

#include "camera.h"
#include "vector2.h"

#include <SDL.h>

extern SDL_Texture* tex_bullet; //全局变量存储资源对象，子弹的纹理

class Bullet {
public:
	Bullet(double angle) {
		this->angle = angle;

		double radians = angle * 3.14159265 / 180;
		velocity.x = (float)std::cos(radians) * speed;
		velocity.y = (float)std::sin(radians) * speed; //速度向量
	}

	~Bullet() = default;

	void set_position(const Vector2& position) {
		this->position = position;
	}

	const Vector2& get_position()const {
		return position;
	}

	void on_update(float delta) { //更新
		position += velocity * delta;
		if (position.x <= 0 || position.x >= 1280 || position.y <= 0 || position.y >= 720) {
			is_valid = false;
		}
	}

	void on_render(const Camera& camera)const {
		const SDL_FRect rect_bullet = { position.x - 4,position.y - 2,8,4 };
		camera.render_texture(tex_bullet, nullptr, &rect_bullet, angle, nullptr);
	}

	//检查子弹失效
	void on_hit() {
		is_valid = false;
	}
	bool can_remove()const {
		return !is_valid;
	}

private:
	double angle = 0;
	Vector2 position;
	Vector2 velocity;
	bool is_valid = true;
	float speed = 800.0f;
};

#endif // !_BULLET_H_

