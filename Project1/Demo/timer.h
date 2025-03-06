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
	//�������ö�ʱ�����δ���/ѭ������
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
	//���¶�ʱ����״̬���ж��Ƿ�ô�����ʱ�¼�
	void on_update(float delta) {
		if (paused) {
			return;
		}
		pass_time += delta;
		if (pass_time >= wait_time) {
			bool can_shot = (!one_shot || (one_shot && !shotted));
			//one_shot: flase��ѭ��������can_shot��Ϊ��������
			//one_shot��true�����δ�����shottedΪfalse������������
			shotted = true;
			if (can_shot && on_timeout) {
				on_timeout();
			}
			pass_time -= wait_time;
		}
	}



private:
	float pass_time; 
	//��¼�Ѿ�������ʱ�䡣ÿ�ε��� on_update ʱ
	//���ۼ���һ�� delta��ʱ����������
	float wait_time; 
	//��ʱ��������ʱ��
	//�� pass_time �ﵽ wait_time ʱ����ʱ���ᴥ���¼���
	bool shotted; 
	//��Ƕ�ʱ���Ƿ��Ѿ��������¼�
	//������������ж��Ƿ��Ѿ�������һ���¼�
	//������ "one-shot" ģʽ���ظ�������
	bool one_shot; 
	//�Ƿ��ǵ��δ�����
	bool paused;
	std::function<void()> on_timeout; 
	//����һ���ص��������ڶ�ʱ����ʱ��pass_time >= wait_time��ʱ����
	// std::function<void()> �����û������κ�û�з���ֵ�ĺ�����Ϊ��ʱ�ص���

};


#endif // !_TIMER_H_

