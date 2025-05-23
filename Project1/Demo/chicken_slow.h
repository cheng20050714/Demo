#ifndef _CHICKEN_SLOW_H_
#define _CHICKEN_SLOW_H_

#include "chicken.h"

extern Atlas atlas_chicken_slow;

class ChickenSlow : public Chicken {
public:
	ChickenSlow() {
		animation_run.add_frame(&atlas_chicken_slow);
		speed_run = 10.0f;
		//30->15
	}

	~ChickenSlow() = default;
};

#endif