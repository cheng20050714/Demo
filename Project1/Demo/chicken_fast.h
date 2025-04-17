#ifndef _CHICKEN_FAST_H_
#define _CHICKEN_FAST_H_

#include "chicken.h"

extern Atlas atlas_chicken_fast;

class ChickenFast : public Chicken {
public:
	ChickenFast() {
		animation_run.add_frame(&atlas_chicken_fast);
		speed_run = 30.0f;
		//80->30
	}

	~ChickenFast() = default;
};

#endif