#define SDL_MAIN_HANDLED


#include "atlas.h"
#include "camera.h"
#include "bullet.h"
#include "chicken.h"
#include "chicken_fast.h"
#include "chicken_slow.h"
#include "chicken_medium.h"
#include "boss_1.h"
#include "boss_2.h"
#include "boss_3.h"

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
Atlas atlas_boss_explosion;
Atlas atlas_chicken_fast;
Atlas atlas_chicken_medium;
Atlas atlas_chicken_slow;
Atlas atlas_explosion;
Atlas atlas_boss_1;
Atlas atlas_boss_2;
Atlas atlas_boss_3;


Mix_Music* music_bgm = nullptr;
Mix_Music* music_loss = nullptr;

Mix_Chunk* sound_hurt = nullptr;
Mix_Chunk* sound_fire_1 = nullptr;
Mix_Chunk* sound_fire_2 = nullptr;
Mix_Chunk* sound_fire_3 = nullptr;
Mix_Chunk* sound_explosion = nullptr;

TTF_Font* font = nullptr;
TTF_Font* font_large = nullptr;




//与游戏逻辑有关的变量定义
int hp = 10;
int score = 0;
std::vector<Bullet> bullet_list;
std::vector<Chicken*> chicken_list;

double num_per_gen = 1.5;
Timer timer_generate;  //僵尸鸡生成定时器
Timer timer_increase_num_per_gen; //增加每次生成数量定时器
Timer timer_auto_fire; // 自动开火定时器
bool is_barrel_firing = false; // 是否正在开火的标志
float barrel_fire_time = 0;    // 开火动画已播放时间
const float BARREL_FIRE_DURATION = 0.3f; // 开火动画持续时间


Vector2 pos_crosshair;
double angle_barrel = 0; //炮管旋转角度
const Vector2 pos_battery = { 640,600 }; //炮台基座中心位置
const Vector2 pos_barrel = { 582,585 }; //炮管无旋转默认位置
const SDL_FPoint center_barrel = { 48,15 }; //炮管旋转中心坐标

//bool is_cool_down = true; //是否冷却结束
//bool is_fire_key_down = false;
Animation animation_barrel_fire; //炮管开火动画


//单词模式全局变量
std::map<std::string, std::string> word_dictionary; // 英文单词到中文释义的映射
std::string current_word;               // 当前要拼写的单词
std::string current_definition;         // 当前单词的中文释义
std::string player_input;               // 玩家当前输入
bool is_spelling_game_active = false;   // 拼写游戏是否激活
int correct_letters = 0;                // 已正确输入的字母数
Timer timer_spelling_game;

//Boss模式全局变量
Boss* boss = nullptr; // 用于管理 Boss 的指针
bool is_boss_active = false; // 标志 Boss 是否激活
// 如果得分达到 100 且 Boss2 和 Boss3 尚未激活，则生成它们
Boss* boss2 = nullptr;
Boss* boss3 = nullptr;
bool are_boss2_and_boss3_active = false;



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
	atlas_chicken_fast.load(renderer, "../resources/chiikawa_3/chiikawa_3-%d.png", 5);
	atlas_chicken_medium.load(renderer, "../resources/chiikawa_1/chiikawa_1-%d.png", 17);
	atlas_chicken_slow.load(renderer, "../resources/chiikawa_2/chiikawa_2-%d.png", 21);
	atlas_explosion.load(renderer, "../resources/explosion_%d.png", 5);
	atlas_boss_explosion.load(renderer, "../resources/boss_explosion/explosion_%d.png", 5);
    //atlas_boss_1.load(renderer, "../resources/boss_1_%d.png", 3);
    atlas_boss_1.load(renderer, "../resources/joke_bear/joke_bear_1-%d.png",11);
    atlas_boss_2.load(renderer, "../resources/joke_bear_2/joke_bear_2-%d.png", 11);
	atlas_boss_3.load(renderer, "../resources/joke_bear_3/joke_bear_3-%d.png", 8);




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
    font_large = TTF_OpenFont("../resources/IPix.ttf", 40); // 新增大字体


	//加载words_list
	std::ifstream file("../resources/words_list.txt");


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
	timer_generate.set_on_timeout([&]() {
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

	// 播放背景音乐
	Mix_PlayMusic(music_bgm, -1);

	// 初始化拼写游戏
	if (!word_dictionary.empty()) {
		auto it = word_dictionary.begin();
		std::advance(it, rand() % word_dictionary.size());
		current_word = it->first;
		current_definition = it->second;
		is_spelling_game_active = true;
	}
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
    // 如果得分达到 10 且 Boss1 尚未激活，则生成 Boss1
    if (score >= 10 && score < 20 && !is_boss_active) {
        boss = new Boss1(); // 创建 Boss1
        is_boss_active = true;
    }



    if (score >= 70 &&score<=80 && !are_boss2_and_boss3_active) {
        boss2 = new Boss2(); // 创建 Boss2
        boss3 = new Boss3(); // 创建 Boss3
        are_boss2_and_boss3_active = true;
    }

    // 更新 Boss1
    if (is_boss_active && boss) {
        boss->on_update(delta);

        // 如果 Boss1 死亡，清理资源
        if (boss->can_remove()) {
            delete boss;
            boss = nullptr;
            is_boss_active = false;
        }
    }

    // 更新 Boss2 和 Boss3
    if (are_boss2_and_boss3_active) {
        if (boss2) {
            boss2->on_update(delta);
            if (boss2->can_remove()) {
                delete boss2;
                boss2 = nullptr;
            }
        }

        if (boss3) {
            boss3->on_update(delta);
            if (boss3->can_remove()) {
                delete boss3;
                boss3 = nullptr;
            }
        }

        // 如果 Boss2 和 Boss3 都被移除，则标记为不活跃
        if (!boss2 && !boss3) {
            are_boss2_and_boss3_active = false;
        }
    }

    // 自动瞄准最近的僵尸鸡
    if (!chicken_list.empty()) {
        // 找到最近的鸡（这里用 Y 坐标最大的，也就是最靠近底部的）
        Chicken* target_chicken = *std::min_element(chicken_list.begin(), chicken_list.end(),
            [](Chicken* a, Chicken* b) {
                return a->get_position().y > b->get_position().y;
            });

        // 计算目标方向
        Vector2 target_direction = target_chicken->get_position() - pos_battery;
        angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
    }
    // 自动瞄准 Boss
    else if (is_boss_active && boss) {
        // 获取 Boss 的位置
        const Vector2& boss_position = boss->get_position();

        // 计算目标方向
        Vector2 target_direction = boss_position - pos_battery;

        // 计算炮管的旋转角度
        angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
    }
    else if (are_boss2_and_boss3_active) {
        // 优先瞄准 Boss2 和 Boss3
        if (boss2) {
            const Vector2& boss2_position = boss2->get_position();
            Vector2 target_direction = boss2_position - pos_battery;
            angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
        }
        else if (boss3) {
            const Vector2& boss3_position = boss3->get_position();
            Vector2 target_direction = boss3_position - pos_battery;
            angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
        }
    }

    // 更新鸡和子弹的逻辑保持不变
    if (!is_boss_active && !are_boss2_and_boss3_active) {
        timer_generate.on_update(delta);
        timer_increase_num_per_gen.on_update(delta);

        for (Chicken* chicken : chicken_list) {
            chicken->on_update(delta);
        }
    }

    if (is_barrel_firing) {
        barrel_fire_time += delta;
        camera->shake(3.0f, 0.1f);
        animation_barrel_fire.on_update(delta);

        if (barrel_fire_time >= BARREL_FIRE_DURATION) {
            is_barrel_firing = false;
            barrel_fire_time = 0;
        }
    }

    for (Bullet& bullet : bullet_list) {
        bullet.on_update(delta);

        // 检查子弹是否击中 Boss1
        if (is_boss_active && boss) {
            const Vector2& pos_bullet = bullet.get_position();
            const Vector2& pos_boss = boss->get_position();
            static const Vector2 size_boss = { 200, 200 };
            if (pos_bullet.x >= pos_boss.x - size_boss.x / 2 &&
                pos_bullet.x <= pos_boss.x + size_boss.x / 2 &&
                pos_bullet.y >= pos_boss.y - size_boss.y / 2 &&
                pos_bullet.y <= pos_boss.y + size_boss.y / 2) {
                bullet.on_hit();
                boss->on_hurt();
            }
        }

        // 检查子弹是否击中 Boss2
        if (boss2) {
            const Vector2& pos_bullet = bullet.get_position();
            const Vector2& pos_boss2 = boss2->get_position();
            static const Vector2 size_boss2 = { 200, 200 };
            if (pos_bullet.x >= pos_boss2.x - size_boss2.x / 2 &&
                pos_bullet.x <= pos_boss2.x + size_boss2.x / 2 &&
                pos_bullet.y >= pos_boss2.y - size_boss2.y / 2 &&
                pos_bullet.y <= pos_boss2.y + size_boss2.y / 2) {
                bullet.on_hit();
                boss2->on_hurt();
            }
        }

        // 检查子弹是否击中 Boss3
        if (boss3) {
            const Vector2& pos_bullet = bullet.get_position();
            const Vector2& pos_boss3 = boss3->get_position();
            static const Vector2 size_boss3 = { 200, 200 };
            if (pos_bullet.x >= pos_boss3.x - size_boss3.x / 2 &&
                pos_bullet.x <= pos_boss3.x + size_boss3.x / 2 &&
                pos_bullet.y >= pos_boss3.y - size_boss3.y / 2 &&
                pos_bullet.y <= pos_boss3.y + size_boss3.y / 2) {
                bullet.on_hit();
                boss3->on_hurt();
            }
        }
    }

    //更新僵尸鸡列表并处理子弹碰撞
    for (Chicken* chicken : chicken_list) {
        chicken->on_update(delta);

        //对每个子弹依次检测
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

    // 移除这里的重复代码，因为已经在开火状态处理中处理了
    // 不开火时不需要摄像机抖动
    // camera->shake(3.0f, 0.1f);
    // animation_barrel_fire.on_update(delta);

    //更新摄像机状态，发生抖动
    camera->on_update(delta);

    //检查游戏是否结束
    if (hp <= 0) {
        is_quit = true;
        Mix_HaltMusic();
        Mix_PlayMusic(music_loss, 0);

        std::string msg = u8"Final score:" + std::to_string(score);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, u8"End!!", msg.c_str(), window);
    }
}


// 修改渲染函数，移除准心渲染
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

    // 绘制 Boss
    if (is_boss_active && boss) {
        boss->on_render(camera);
    }

    if (are_boss2_and_boss3_active) {
        if (boss2) {
            boss2->on_render(camera);
        }
        if (boss3) {
            boss3->on_render(camera);
        }
    }


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
            pos_barrel.x, pos_barrel.y,
            (float)width_barrel, (float)height_barrel
        };

        if (is_barrel_firing) {
            // 如果正在开火，显示开火动画
            animation_barrel_fire.set_rotation(angle_barrel);
            animation_barrel_fire.set_position({ pos_battery.x + 40,
            pos_battery.y + 10}); // 确保位置正确
            animation_barrel_fire.on_render(camera);
        }
        else {
            // 如果没有开火，显示静态炮管
            camera.render_texture(tex_barrel_idle, nullptr, &rect_barrel, angle_barrel, &center_barrel);
        }
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
        if (suf_score_bg && suf_score_fg) {
            SDL_Texture* tex_score_bg = SDL_CreateTextureFromSurface(renderer, suf_score_bg);
            SDL_Texture* tex_score_fg = SDL_CreateTextureFromSurface(renderer, suf_score_fg);
            if (tex_score_bg && tex_score_fg) {
                SDL_Rect rect_dst_score = { 1280 - suf_score_bg->w - 15, 15, suf_score_bg->w, suf_score_bg->h };
                SDL_RenderCopy(renderer, tex_score_bg, nullptr, &rect_dst_score);
                rect_dst_score.x -= 2;
                rect_dst_score.y -= 2;
                SDL_RenderCopy(renderer, tex_score_fg, nullptr, &rect_dst_score);
                SDL_DestroyTexture(tex_score_bg);
                SDL_DestroyTexture(tex_score_fg);
            }
            SDL_FreeSurface(suf_score_bg);
            SDL_FreeSurface(suf_score_fg);
        }
    }

    // 移除绘制准心的代码块

    // 绘制中文释义和玩家输入
    if (is_spelling_game_active) {
        int screen_width = 1280;
        int screen_height = 720;

        SDL_Surface* suf_definition_bg = TTF_RenderUTF8_Blended(font_large, current_definition.c_str(), { 55, 55, 55, 255 });
        SDL_Surface* suf_definition_fg = TTF_RenderUTF8_Blended(font_large, current_definition.c_str(), { 255, 255, 255, 255 });
        if (suf_definition_bg && suf_definition_fg) {
            SDL_Texture* tex_definition_bg = SDL_CreateTextureFromSurface(renderer, suf_definition_bg);
            SDL_Texture* tex_definition_fg = SDL_CreateTextureFromSurface(renderer, suf_definition_fg);
            if (tex_definition_bg && tex_definition_fg) {
                SDL_Rect rect_dst_definition = {
                    (screen_width - suf_definition_bg->w) / 2, // 居中
                    (screen_height / 2) - 50,                 // 上移一些
                    suf_definition_bg->w,
                    suf_definition_bg->h
                };
                SDL_RenderCopy(renderer, tex_definition_bg, nullptr, &rect_dst_definition);
                rect_dst_definition.x -= 2;
                rect_dst_definition.y -= 2;
                SDL_RenderCopy(renderer, tex_definition_fg, nullptr, &rect_dst_definition);
                SDL_DestroyTexture(tex_definition_bg);
                SDL_DestroyTexture(tex_definition_fg);
            }
            SDL_FreeSurface(suf_definition_bg);
            SDL_FreeSurface(suf_definition_fg);
        }

        // 绘制玩家输入
        SDL_Surface* suf_input_bg = TTF_RenderUTF8_Blended(font_large, player_input.c_str(), { 55, 55, 55, 255 });
        SDL_Surface* suf_input_fg = TTF_RenderUTF8_Blended(font_large, player_input.c_str(), { 255, 255, 255, 255 });
        if (suf_input_bg && suf_input_fg) {
            SDL_Texture* tex_input_bg = SDL_CreateTextureFromSurface(renderer, suf_input_bg);
            SDL_Texture* tex_input_fg = SDL_CreateTextureFromSurface(renderer, suf_input_fg);
            if (tex_input_bg && tex_input_fg) {
                SDL_Rect rect_dst_input = {
                    (screen_width - suf_input_bg->w) / 2, // 居中
                    (screen_height / 2) + 30,            // 下移一些
                    suf_input_bg->w,
                    suf_input_bg->h
                };
                SDL_RenderCopy(renderer, tex_input_bg, nullptr, &rect_dst_input);
                rect_dst_input.x -= 2;
                rect_dst_input.y -= 2;
                SDL_RenderCopy(renderer, tex_input_fg, nullptr, &rect_dst_input);
                SDL_DestroyTexture(tex_input_bg);
                SDL_DestroyTexture(tex_input_fg);
            }
            SDL_FreeSurface(suf_input_bg);
            SDL_FreeSurface(suf_input_fg);
        }
    }
}

// 修改主循环，移除鼠标事件处理
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
                // 移除鼠标移动事件处理
            case SDL_KEYDOWN:
                if (is_spelling_game_active) {
                    char key = event.key.keysym.sym;
                    // 检查按键是否是小写字母
                    if (key >= 'a' && key <= 'z') {
                        // 检查当前输入位置的正确字母
                        if (player_input.length() < current_word.length() &&
                            key == current_word[player_input.length()]) {
                            // 输入正确，添加到玩家输入
                            player_input += key;

                            // 开始开火动画
                            is_barrel_firing = true;
                            barrel_fire_time = 0;
                            animation_barrel_fire.reset();

                            // 生成子弹
                            static const float length_barrel = 105;  // 炮管长度
                            static const Vector2 pos_barrel_center = { 640,610 };  // 炮管锚点中心位置

                            bullet_list.emplace_back(angle_barrel); // 构造新的子弹对象
                            Bullet& bullet = bullet_list.back();
                            double angle_bullet = angle_barrel;
                            double radians = angle_bullet * 3.14159265 / 180;
                            Vector2 bullet_direction = { (float)std::cos(radians),(float)std::sin(radians) };
                            bullet.set_position(pos_barrel_center + bullet_direction * length_barrel); // 重设子弹位置

                            // 播放开火音效
                            switch (rand() % 3) {
                            case 0: Mix_PlayChannel(-1, sound_fire_1, 0); break;
                            case 1: Mix_PlayChannel(-1, sound_fire_2, 0); break;
                            case 2: Mix_PlayChannel(-1, sound_fire_3, 0); break;
                            }

                            // 如果完全拼写正确，选择下一个单词
                            if (player_input == current_word) {
                                // 得分增加
                                score += 10;

                                // 延迟一下再切换单词，让玩家看到完整单词
                                SDL_Delay(100);

                                // 清空玩家输入
                                player_input.clear();

                                // 选择下一个单词
                                auto it = word_dictionary.begin();
                                std::advance(it, rand() % word_dictionary.size());
                                current_word = it->first;
                                current_definition = it->second;
                            }
                        }
                        // 如果输入错误的字母，不做任何处理
                    }
                }
                break;
                // 移除鼠标按钮事件处理
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
}




int main() {
	init();
	mainloop();
	deinit();

	return 0;
}