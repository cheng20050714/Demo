#define SDL_MAIN_HANDLED


#include "atlas.h"
#include "camera.h"
#include "bullet.h"
#include "chicken.h"
#include "chicken_fast.h"
#include "chicken_slow.h"
#include "chicken_medium.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include <fstream>
#include <map>
#include <utility>


//游戏中需要的资源变量
Camera* camera = nullptr;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool is_quit = false;

SDL_Texture* tex_heart = nullptr;
SDL_Texture* tex_bullet = nullptr;
SDL_Texture* tex_battery = nullptr;
SDL_Texture* tex_crosshair = nullptr;
SDL_Texture* tex_background = nullptr;
SDL_Texture* tex_barrel_idle = nullptr;

Atlas atlas_barrel_fire;
Atlas atlas_chicken_fast;
Atlas atlas_chicken_medium;
Atlas atlas_chicken_slow;
Atlas atlas_explosion;

Mix_Music* music_bgm = nullptr;
Mix_Music* music_loss = nullptr;

Mix_Chunk* sound_hurt = nullptr;
Mix_Chunk* sound_fire_1 = nullptr;
Mix_Chunk* sound_fire_2 = nullptr;
Mix_Chunk* sound_fire_3 = nullptr;
Mix_Chunk* sound_explosion = nullptr;

TTF_Font* font = nullptr;



//与游戏逻辑有关的变量定义
int hp = 10;
int score = 0;
std::vector<Bullet> bullet_list;
std::vector<Chicken*> chicken_list;

int num_per_gen = 2;
Timer timer_generate;  //僵尸鸡生成定时器
Timer timer_increase_num_per_gen; //增加每次生成数量定时器
Timer timer_auto_fire; // 自动开火定时器

Vector2 pos_crosshair;
double angle_barrel = 0; //炮管旋转角度
const Vector2 pos_battery = { 640,600 }; //炮台基座中心位置
const Vector2 pos_barrel = { 592,585 }; //炮管无旋转默认位置
const SDL_FPoint center_barrel = { 48,15 }; //炮管旋转中心坐标

//bool is_cool_down = true; //是否冷却结束
//bool is_fire_key_down = false;
Animation animation_barrel_fire; //炮管开火动画

std::map<std::string, std::string> word_dictionary; // 英文单词到中文释义的映射
std::string current_word;               // 当前要拼写的单词
std::string current_definition;         // 当前单词的中文释义
std::string player_input;               // 玩家当前输入
bool is_spelling_game_active = false;   // 拼写游戏是否激活
int correct_letters = 0;                // 已正确输入的字母数
Timer timer_spelling_game;

//func：加载所有资源
void load_resources() {
	// 加载图片
	tex_heart = IMG_LoadTexture(renderer, "../resources/heart.png");

	tex_bullet = IMG_LoadTexture(renderer, "../resources/bullet.png");

	tex_battery = IMG_LoadTexture(renderer, "../resources/battery.png");

	tex_crosshair = IMG_LoadTexture(renderer, "../resources/crosshair.png");

	tex_background = IMG_LoadTexture(renderer, "../resources/background.png");

	tex_barrel_idle = IMG_LoadTexture(renderer, "../resources/barrel_idle.png");

	// 加载动画帧
	atlas_barrel_fire.load(renderer, "../resources/barrel_fire_%d.png", 3);
	atlas_chicken_fast.load(renderer, "../resources/chicken_fast_%d.png", 4);
	atlas_chicken_medium.load(renderer, "../resources/chicken_medium_%d.png", 6);
	atlas_chicken_slow.load(renderer, "../resources/chicken_slow_%d.png", 8);
	atlas_explosion.load(renderer, "../resources/explosion_%d.png", 5);

	// 加载音乐
	music_bgm = Mix_LoadMUS("../resources/bgm.mp3");

	music_loss = Mix_LoadMUS("../resources/loss.mp3");

	// 加载音效
	sound_hurt = Mix_LoadWAV("../resources/hurt.wav");

	sound_fire_1 = Mix_LoadWAV("../resources/fire_1.wav");

	sound_fire_2 = Mix_LoadWAV("../resources/fire_2.wav");

	sound_fire_3 = Mix_LoadWAV("../resources/fire_3.wav");

	sound_explosion = Mix_LoadWAV("../resources/explosion.wav");

	// 加载字体
	font = TTF_OpenFont("../resources/IPix.ttf", 28);

	//加载words_list
	std::ifstream file("../resources/words_list.txt");
	if (!file.is_open()) {
		std::cerr << "无法打开单词文件！" << std::endl;
		return;
	}

	std::string english, chinese;
	while (std::getline(file, english)) {
		if (std::getline(file, chinese)) {
			word_dictionary[english] = chinese;
		}
	}
	file.close();
}


//func：释放所有资源
void unload_resources() {
	SDL_DestroyTexture(tex_heart);
	SDL_DestroyTexture(tex_bullet);
	SDL_DestroyTexture(tex_battery);
	SDL_DestroyTexture(tex_crosshair);
	SDL_DestroyTexture(tex_background);
	SDL_DestroyTexture(tex_barrel_idle);

	Mix_FreeMusic(music_bgm);
	Mix_FreeMusic(music_loss);

	Mix_FreeChunk(sound_hurt);
	Mix_FreeChunk(sound_fire_1);
	Mix_FreeChunk(sound_fire_2);
	Mix_FreeChunk(sound_fire_3);
	Mix_FreeChunk(sound_explosion);
}


//func: 初始化所有资源
void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3);
	TTF_Init();

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_AllocateChannels(32); //支持同一时刻播放更多音频对象

	window = SDL_CreateWindow(u8"demo",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	


	SDL_ShowCursor(SDL_DISABLE); //光标不可见

	load_resources();

	camera = new Camera(renderer);

	timer_generate.set_one_shot(false);
	timer_generate.set_wait_time(1.5f);
	timer_generate.set_on_timeout([&](){
		for (int i = 0; i < num_per_gen; i++) {
			int val = rand() % 100;
			Chicken* chicken = nullptr;
			if (val < 50)
				chicken = new ChickenSlow();
			else if (val < 80) 
				chicken = new ChickenMedium();
			else
				chicken = new ChickenFast();
			chicken_list.push_back(chicken);
		}
	});


	animation_barrel_fire.add_frame(&atlas_barrel_fire);
	animation_barrel_fire.set_position(pos_battery);
	animation_barrel_fire.set_center(center_barrel);
	animation_barrel_fire.set_interval(0.1f); 

	// 设置自动开火定时器
	timer_auto_fire.set_one_shot(false);
	timer_auto_fire.set_wait_time(0.5f);
	timer_auto_fire.set_on_timeout([&]() {
		if (!chicken_list.empty()) {
			// 找到最靠近边界的鸡
			Chicken* target_chicken = *std::min_element(chicken_list.begin(), chicken_list.end(),
				[](Chicken* a, Chicken* b) {
					return a->get_position().y > b->get_position().y;
				});

			// 计算目标方向
			Vector2 target_direction = target_chicken->get_position() - pos_battery;
			angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;

			// 生成子弹
			animation_barrel_fire.reset();

			static const float length_barrel = 105;  // 炮管长度
			static const Vector2 pos_barrel_center = { 640,610 };  // 炮管锚点中心位置

			bullet_list.emplace_back(angle_barrel); // 构造新的子弹对象
			Bullet& bullet = bullet_list.back();
			double angle_bullet = angle_barrel + (rand() % 30 - 15); // 弹道随机偏移
			double radians = angle_bullet * 3.14159265 / 180;
			Vector2 bullet_direction = { (float)std::cos(radians),(float)std::sin(radians) };
			bullet.set_position(pos_barrel_center + bullet_direction * length_barrel); // 重设子弹位置

			// 播放开火音效
			switch (rand() % 3) {
			case 0: Mix_PlayChannel(-1, sound_fire_1, 0); break;
			case 1: Mix_PlayChannel(-1, sound_fire_2, 0); break;
			case 2: Mix_PlayChannel(-1, sound_fire_3, 0); break;
			}
		}
		});

	// 播放背景音乐
	Mix_PlayMusic(music_bgm, -1);
}

void deinit() {
	delete camera;

	unload_resources();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

//func：更新
void on_update(float delta) {
	//更新所有定时器
	timer_generate.on_update(delta);
	timer_increase_num_per_gen.on_update(delta);
	timer_auto_fire.on_update(delta); // 更新自动开火定时器


	//更新子弹列表：位置更新
	for (Bullet& bullet : bullet_list) {
		bullet.on_update(delta);
	}

	//更新僵尸鸡列表并处理子弹碰撞
	for (Chicken* chicken : chicken_list) {
		chicken->on_update(delta);

		//对每个子弹依次检测
		//可否考虑优化
		for (Bullet& bullet : bullet_list) {
			if (!chicken->check_alive() || bullet.can_remove()) 
				continue;

			const Vector2& pos_bullet = bullet.get_position();
			const Vector2& pos_chicken = chicken->get_position();
			static const Vector2 size_chicken = { 30,40 };
			if (pos_bullet.x >= pos_chicken.x - size_chicken.x / 2
				&& pos_bullet.x <= pos_chicken.x + size_chicken.x / 2
				&& pos_bullet.y >= pos_chicken.y - size_chicken.y / 2
				&& pos_bullet.y <= pos_chicken.y + size_chicken.y / 2) {
				score += 1;
				bullet.on_hit();
				chicken->on_hurt();//is_alive:false
			}
		}

		if (!chicken->check_alive())
			continue;

		//鸡超过底部，hp--，播放音效
		if (chicken->get_position().y >= 720) {
			chicken->make_invalid();
			Mix_PlayChannel(-1, sound_hurt, 0);
			hp -= 1;
		}
	}

	//删除所有失效对象
	bullet_list.erase(std::remove_if(
		bullet_list.begin(), bullet_list.end(),
		[](const Bullet& bullet) {
			return bullet.can_remove();
		}),
		bullet_list.end());

	chicken_list.erase(std::remove_if(
		chicken_list.begin(), chicken_list.end(),
		[](Chicken* chicken) {
			bool can_remove = chicken->can_remove();
			if (can_remove)delete chicken;
			return can_remove;
		}),
		chicken_list.end());

	std::sort(chicken_list.begin(), chicken_list.end(),
		[](const Chicken* chicken_1, const Chicken* chicken_2)
		{return chicken_1->get_position().y < chicken_2->get_position().y; });

	//正在开火，camera->shake()
	/*
	if (!is_cool_down) {
		camera->shake(3.0f, 0.1f);
		animation_barrel_fire.on_update(delta); 
	}
	*/
	//改为自动开火
	camera->shake(3.0f, 0.1f);
	animation_barrel_fire.on_update(delta);

	//处理开火瞬间逻辑（没有开火&&按下开火键）
	/*
	if (is_cool_down && is_fire_key_down) {
		animation_barrel_fire.reset();
		is_cool_down = false;

		static const float length_barrel = 105;  //炮管长度
		static const Vector2 pos_barrel_center = { 640,610 };  //炮管锚点中心位置

		bullet_list.emplace_back(angle_barrel);//构造新的子弹对象
		Bullet& bullet = bullet_list.back();
		double angle_bullet = angle_barrel + (rand() % 30 - 15);//弹道随机偏移
		double radians = angle_bullet * 3.14159265 / 180;
		Vector2 direciton = { (float)std::cos(radians),(float)std::sin(radians) };
		bullet.set_position(pos_barrel_center + direciton * length_barrel);//重设子弹位置



		switch (rand() % 3) {
		case 0:Mix_PlayChannel(-1, sound_fire_1, 0); break;
		case 1:Mix_PlayChannel(-1, sound_fire_2, 0); break;
		case 2:Mix_PlayChannel(-1, sound_fire_3, 0); break;
		}
	}
	*/

	//正在开火，camera->shake()
	camera->shake(3.0f, 0.1f);
	animation_barrel_fire.on_update(delta);

	//更新摄像机状态，发生抖动
	camera->on_update(delta);

	//检查游戏是否结束
	if (hp <= 0) {
		is_quit = true;
		Mix_HaltMusic();
		Mix_PlayMusic(music_loss,0);

		std::string msg = u8"Final score:" + std::to_string(score);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, u8"End!!", msg.c_str(), window);
	}

}

void on_render(const Camera& camera) {

	//绘制背景图
	{
		int width_bg, height_bg;
		SDL_QueryTexture(tex_background, nullptr, nullptr, &width_bg, &height_bg);
		const SDL_FRect rect_background = {
			(1280 - width_bg) / 2.0f,
			(720 - height_bg) / 2.0f,
			(float)width_bg,(float)height_bg,
		};
		camera.render_texture(tex_background, nullptr, &rect_background, 0, nullptr);
	}

	//绘制鸡
	for (Chicken* chicken : chicken_list)
		chicken->on_render(camera);

	//绘制子弹
	for (const Bullet& bullet : bullet_list)
		bullet.on_render(camera);


	//绘制炮台
	{

		int width_battery, height_battery;
		SDL_QueryTexture(tex_battery, nullptr, nullptr, &width_battery, &height_battery);
		const SDL_FRect rect_battery = {
			pos_battery.x - width_battery / 2.0f,
			pos_battery.y - height_battery / 2.0f,
			(float)width_battery,(float)height_battery
		};
		camera.render_texture(tex_battery, nullptr, &rect_battery, 0, nullptr);

		//绘制炮管
		int width_barrel, height_barrel;
		SDL_QueryTexture(tex_barrel_idle, nullptr, nullptr, &width_barrel, &height_barrel);
		const SDL_FRect rect_barrel = {
			pos_barrel.x,pos_barrel.y,
			(float)width_barrel,(float)height_barrel
		};
		/*
		if (is_cool_down)
			camera.render_texture(tex_barrel_idle, nullptr, &rect_barrel, angle_barrel, &center_barrel);
		else {
			animation_barrel_fire.set_rotation(angle_barrel);
			animation_barrel_fire.on_render(camera);
		}
		*/
		// 由于现在总是使用开火动画，可以简化为
		animation_barrel_fire.set_rotation(angle_barrel);
		animation_barrel_fire.on_render(camera);
	}


	//绘制生命值
	{
		int width_heart, height_heart;
		SDL_QueryTexture(tex_heart, nullptr, nullptr, &width_heart, &height_heart);
		for (int i = 0; i < hp; i++)
		{
			const SDL_Rect rect_dst =
			{
				15 + (width_heart + 10) * i, 15,
				width_heart, height_heart
			};
			SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_dst);
		}
	}

	{
		// 绘制游戏得分
		std::string str_score = "SCORE: " + std::to_string(score);
		SDL_Surface* suf_score_bg = TTF_RenderUTF8_Blended(font, str_score.c_str(), { 55, 55, 55, 255 });
		SDL_Surface* suf_score_fg = TTF_RenderUTF8_Blended(font, str_score.c_str(), { 255, 255, 255, 255 });
		SDL_Texture* tex_score_bg = SDL_CreateTextureFromSurface(renderer, suf_score_bg);
		SDL_Texture* tex_score_fg = SDL_CreateTextureFromSurface(renderer, suf_score_fg);
		SDL_Rect rect_dst_score = { 1280 - suf_score_bg->w - 15, 15, suf_score_bg->w, suf_score_bg->h };
		SDL_RenderCopy(renderer, tex_score_bg, nullptr, &rect_dst_score);
		rect_dst_score.x -= 2;
		rect_dst_score.y -= 2;
		SDL_RenderCopy(renderer, tex_score_fg, nullptr, &rect_dst_score);
		SDL_DestroyTexture(tex_score_bg);
		SDL_DestroyTexture(tex_score_fg);
		SDL_FreeSurface(suf_score_bg);
		SDL_FreeSurface(suf_score_fg);

	}

	//绘制准心
	{
		int width_crosshair, height_crosshair;
		SDL_QueryTexture(tex_crosshair, nullptr, nullptr, &width_crosshair, &height_crosshair);
		const SDL_FRect rect_crosshair =
		{
			pos_crosshair.x - width_crosshair / 2.0f,
			pos_crosshair.y - height_crosshair / 2.0f,
			(float)width_crosshair, (float)height_crosshair
		};
		camera.render_texture(tex_crosshair, nullptr, &rect_crosshair, 0, nullptr);

	}

}

void mainloop() {
	using namespace std::chrono;
	SDL_Event event;

	const nanoseconds frame_duration(1000000000 / 144);
	steady_clock::time_point last_tick = steady_clock::now();

	while (!is_quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				is_quit = true;
				break;
			case SDL_MOUSEMOTION: {
				pos_crosshair.x = (float)event.motion.x;
				pos_crosshair.y = (float)event.motion.y;
				Vector2 direction = pos_crosshair - pos_battery;
				angle_barrel = std::atan2(direction.y, direction.x) * 180 / 3.14159265;
			}
				break;
			/*
			case SDL_MOUSEBUTTONDOWN:
				is_fire_key_down = true;
				break;
			case SDL_MOUSEBUTTONUP:
				is_fire_key_down = false;
				break;
				*/

			// 可以完全移除这些代码，因为不再需要处理手动开火
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			}
		}
		steady_clock::time_point frame_start = steady_clock::now();
		duration<float> delta = duration<float>(frame_start - last_tick);

		on_update(delta.count());
		on_render(*camera);
		SDL_RenderPresent(renderer);

		last_tick = frame_start;
		nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
		if (sleep_duration > nanoseconds(0)) {
			std::this_thread::sleep_for(sleep_duration);
		}
	}

}//游戏主循环

int main() {
	init();
	mainloop();
	deinit();

	return 0;
}