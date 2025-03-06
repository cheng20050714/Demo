#ifndef _TIMER_H_
#define _TIMER_H_

#include<functional>

class Timer {
public:
	Timer()=default;
	~Timer() = default;

	void restart() {
		pass_time = 0;
		shotted = false;
	}
	void set_wait_time(float val) {
		wait_time = val;
	}
	//用以设置定时器单次触发/循环触发
	void set_one_shot(bool flag) {
		one_shot = flag;
	}
	void set_on_timeout(std::function<void()>on_timeout) {
		this->on_timeout = on_timeout;
	}

	void pause() {
		paused = true;
	}
	void resume() {
		paused = false;
	}
	//更新定时器的状态并判断是否该触发超时事件
	void on_update(float delta) {
		if (paused) {
			return;
		}
		pass_time += delta;
		if (pass_time >= wait_time) {
			bool can_shot = (!one_shot || (one_shot && !shotted));
			//one_shot: flase，循环触发。can_shot则为允许触发。
			//one_shot：true，单次触发，shotted为false，才允许触发。
			shotted = true;
			if (can_shot && on_timeout) {
				on_timeout();
			}
			pass_time -= wait_time;
		}
	}



private:
	float pass_time; 
	//记录已经经过的时间。每次调用 on_update 时
	//会累加上一个 delta（时间增量）。
	float wait_time; 
	//定时器的设置时间
	//当 pass_time 达到 wait_time 时，定时器会触发事件。
	bool shotted; 
	//标记定时器是否已经触发过事件
	//这个变量用于判断是否已经触发过一次事件
	//避免在 "one-shot" 模式下重复触发。
	bool one_shot; 
	//是否是单次触发。
	bool paused;
	std::function<void()> on_timeout; 
	//这是一个回调函数，在定时器超时（pass_time >= wait_time）时调用
	// std::function<void()> 允许用户传入任何没有返回值的函数作为超时回调。

};


#endif // !_TIMER_H_

