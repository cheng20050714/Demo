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


//��Ϸ����Ҫ����Դ����
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




//����Ϸ�߼��йصı�������
int hp = 10;
int score = 0;
std::vector<Bullet> bullet_list;
std::vector<Chicken*> chicken_list;

double num_per_gen = 1.5;
Timer timer_generate;  //��ʬ�����ɶ�ʱ��
Timer timer_increase_num_per_gen; //����ÿ������������ʱ��
Timer timer_auto_fire; // �Զ�����ʱ��
bool is_barrel_firing = false; // �Ƿ����ڿ���ı�־
float barrel_fire_time = 0;    // ���𶯻��Ѳ���ʱ��
const float BARREL_FIRE_DURATION = 0.3f; // ���𶯻�����ʱ��


Vector2 pos_crosshair;
double angle_barrel = 0; //�ڹ���ת�Ƕ�
const Vector2 pos_battery = { 640,600 }; //��̨��������λ��
const Vector2 pos_barrel = { 582,585 }; //�ڹ�����תĬ��λ��
const SDL_FPoint center_barrel = { 48,15 }; //�ڹ���ת��������

//bool is_cool_down = true; //�Ƿ���ȴ����
//bool is_fire_key_down = false;
Animation animation_barrel_fire; //�ڹܿ��𶯻�


//����ģʽȫ�ֱ���
std::map<std::string, std::string> word_dictionary; // Ӣ�ĵ��ʵ����������ӳ��
std::string current_word;               // ��ǰҪƴд�ĵ���
std::string current_definition;         // ��ǰ���ʵ���������
std::string player_input;               // ��ҵ�ǰ����
bool is_spelling_game_active = false;   // ƴд��Ϸ�Ƿ񼤻�
int correct_letters = 0;                // ����ȷ�������ĸ��
Timer timer_spelling_game;

//Bossģʽȫ�ֱ���
Boss* boss = nullptr; // ���ڹ��� Boss ��ָ��
bool is_boss_active = false; // ��־ Boss �Ƿ񼤻�
// ����÷ִﵽ 100 �� Boss2 �� Boss3 ��δ�������������
Boss* boss2 = nullptr;
Boss* boss3 = nullptr;
bool are_boss2_and_boss3_active = false;



//func������������Դ
void load_resources() {
	// ����ͼƬ
	tex_heart = IMG_LoadTexture(renderer, "../resources/heart.png");

	tex_bullet = IMG_LoadTexture(renderer, "../resources/bullet.png");

	tex_battery = IMG_LoadTexture(renderer, "../resources/battery.png");

	tex_crosshair = IMG_LoadTexture(renderer, "../resources/crosshair.png");

	tex_background = IMG_LoadTexture(renderer, "../resources/background.png");

	tex_barrel_idle = IMG_LoadTexture(renderer, "../resources/barrel_idle.png");

	// ���ض���֡
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




	// ��������
	music_bgm = Mix_LoadMUS("../resources/bgm.mp3");

	music_loss = Mix_LoadMUS("../resources/loss.mp3");

	// ������Ч
	sound_hurt = Mix_LoadWAV("../resources/hurt.wav");

	sound_fire_1 = Mix_LoadWAV("../resources/fire_1.wav");

	sound_fire_2 = Mix_LoadWAV("../resources/fire_2.wav");

	sound_fire_3 = Mix_LoadWAV("../resources/fire_3.wav");

	sound_explosion = Mix_LoadWAV("../resources/explosion.wav");

	// ��������
	font = TTF_OpenFont("../resources/IPix.ttf", 28);
    font_large = TTF_OpenFont("../resources/IPix.ttf", 40); // ����������


	//����words_list
	std::ifstream file("../resources/words_list.txt");


	std::string english, chinese;
	while (std::getline(file, english)) {
		if (std::getline(file, chinese)) {
			word_dictionary[english] = chinese;
		}
	}
	file.close();
}


//func���ͷ�������Դ
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


//func: ��ʼ��������Դ
void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3);
	TTF_Init();

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_AllocateChannels(32); //֧��ͬһʱ�̲��Ÿ�����Ƶ����

	window = SDL_CreateWindow(u8"demo",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_ShowCursor(SDL_DISABLE); //��겻�ɼ�

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

	// ���ű�������
	Mix_PlayMusic(music_bgm, -1);

	// ��ʼ��ƴд��Ϸ
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


//func������
void on_update(float delta) {
    // ����÷ִﵽ 10 �� Boss1 ��δ��������� Boss1
    if (score >= 10 && score < 20 && !is_boss_active) {
        boss = new Boss1(); // ���� Boss1
        is_boss_active = true;
    }



    if (score >= 70 &&score<=80 && !are_boss2_and_boss3_active) {
        boss2 = new Boss2(); // ���� Boss2
        boss3 = new Boss3(); // ���� Boss3
        are_boss2_and_boss3_active = true;
    }

    // ���� Boss1
    if (is_boss_active && boss) {
        boss->on_update(delta);

        // ��� Boss1 ������������Դ
        if (boss->can_remove()) {
            delete boss;
            boss = nullptr;
            is_boss_active = false;
        }
    }

    // ���� Boss2 �� Boss3
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

        // ��� Boss2 �� Boss3 �����Ƴ�������Ϊ����Ծ
        if (!boss2 && !boss3) {
            are_boss2_and_boss3_active = false;
        }
    }

    // �Զ���׼����Ľ�ʬ��
    if (!chicken_list.empty()) {
        // �ҵ�����ļ��������� Y �������ģ�Ҳ��������ײ��ģ�
        Chicken* target_chicken = *std::min_element(chicken_list.begin(), chicken_list.end(),
            [](Chicken* a, Chicken* b) {
                return a->get_position().y > b->get_position().y;
            });

        // ����Ŀ�귽��
        Vector2 target_direction = target_chicken->get_position() - pos_battery;
        angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
    }
    // �Զ���׼ Boss
    else if (is_boss_active && boss) {
        // ��ȡ Boss ��λ��
        const Vector2& boss_position = boss->get_position();

        // ����Ŀ�귽��
        Vector2 target_direction = boss_position - pos_battery;

        // �����ڹܵ���ת�Ƕ�
        angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265;
    }
    else if (are_boss2_and_boss3_active) {
        // ������׼ Boss2 �� Boss3
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

    // ���¼����ӵ����߼����ֲ���
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

        // ����ӵ��Ƿ���� Boss1
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

        // ����ӵ��Ƿ���� Boss2
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

        // ����ӵ��Ƿ���� Boss3
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

    //���½�ʬ���б������ӵ���ײ
    for (Chicken* chicken : chicken_list) {
        chicken->on_update(delta);

        //��ÿ���ӵ����μ��
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

        //�������ײ���hp--��������Ч
        if (chicken->get_position().y >= 720) {
            chicken->make_invalid();
            Mix_PlayChannel(-1, sound_hurt, 0);
            hp -= 1;
        }
    }

    //ɾ������ʧЧ����
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

    // �Ƴ�������ظ����룬��Ϊ�Ѿ��ڿ���״̬�����д�����
    // ������ʱ����Ҫ���������
    // camera->shake(3.0f, 0.1f);
    // animation_barrel_fire.on_update(delta);

    //���������״̬����������
    camera->on_update(delta);

    //�����Ϸ�Ƿ����
    if (hp <= 0) {
        is_quit = true;
        Mix_HaltMusic();
        Mix_PlayMusic(music_loss, 0);

        std::string msg = u8"Final score:" + std::to_string(score);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, u8"End!!", msg.c_str(), window);
    }
}


// �޸���Ⱦ�������Ƴ�׼����Ⱦ
void on_render(const Camera& camera) {
    //���Ʊ���ͼ
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

    //���Ƽ�
    for (Chicken* chicken : chicken_list)
        chicken->on_render(camera);

    // ���� Boss
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


    //�����ӵ�
    for (const Bullet& bullet : bullet_list)
        bullet.on_render(camera);

    //������̨
    {
        int width_battery, height_battery;
        SDL_QueryTexture(tex_battery, nullptr, nullptr, &width_battery, &height_battery);
        const SDL_FRect rect_battery = {
            pos_battery.x - width_battery / 2.0f,
            pos_battery.y - height_battery / 2.0f,
            (float)width_battery,(float)height_battery
        };
        camera.render_texture(tex_battery, nullptr, &rect_battery, 0, nullptr);

        //�����ڹ�
        int width_barrel, height_barrel;
        SDL_QueryTexture(tex_barrel_idle, nullptr, nullptr, &width_barrel, &height_barrel);
        const SDL_FRect rect_barrel = {
            pos_barrel.x, pos_barrel.y,
            (float)width_barrel, (float)height_barrel
        };

        if (is_barrel_firing) {
            // ������ڿ�����ʾ���𶯻�
            animation_barrel_fire.set_rotation(angle_barrel);
            animation_barrel_fire.set_position({ pos_battery.x + 40,
            pos_battery.y + 10}); // ȷ��λ����ȷ
            animation_barrel_fire.on_render(camera);
        }
        else {
            // ���û�п�����ʾ��̬�ڹ�
            camera.render_texture(tex_barrel_idle, nullptr, &rect_barrel, angle_barrel, &center_barrel);
        }
    }

    //��������ֵ
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
        // ������Ϸ�÷�
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

    // �Ƴ�����׼�ĵĴ����

    // ��������������������
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
                    (screen_width - suf_definition_bg->w) / 2, // ����
                    (screen_height / 2) - 50,                 // ����һЩ
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

        // �����������
        SDL_Surface* suf_input_bg = TTF_RenderUTF8_Blended(font_large, player_input.c_str(), { 55, 55, 55, 255 });
        SDL_Surface* suf_input_fg = TTF_RenderUTF8_Blended(font_large, player_input.c_str(), { 255, 255, 255, 255 });
        if (suf_input_bg && suf_input_fg) {
            SDL_Texture* tex_input_bg = SDL_CreateTextureFromSurface(renderer, suf_input_bg);
            SDL_Texture* tex_input_fg = SDL_CreateTextureFromSurface(renderer, suf_input_fg);
            if (tex_input_bg && tex_input_fg) {
                SDL_Rect rect_dst_input = {
                    (screen_width - suf_input_bg->w) / 2, // ����
                    (screen_height / 2) + 30,            // ����һЩ
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

// �޸���ѭ�����Ƴ�����¼�����
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
                // �Ƴ�����ƶ��¼�����
            case SDL_KEYDOWN:
                if (is_spelling_game_active) {
                    char key = event.key.keysym.sym;
                    // ��鰴���Ƿ���Сд��ĸ
                    if (key >= 'a' && key <= 'z') {
                        // ��鵱ǰ����λ�õ���ȷ��ĸ
                        if (player_input.length() < current_word.length() &&
                            key == current_word[player_input.length()]) {
                            // ������ȷ����ӵ��������
                            player_input += key;

                            // ��ʼ���𶯻�
                            is_barrel_firing = true;
                            barrel_fire_time = 0;
                            animation_barrel_fire.reset();

                            // �����ӵ�
                            static const float length_barrel = 105;  // �ڹܳ���
                            static const Vector2 pos_barrel_center = { 640,610 };  // �ڹ�ê������λ��

                            bullet_list.emplace_back(angle_barrel); // �����µ��ӵ�����
                            Bullet& bullet = bullet_list.back();
                            double angle_bullet = angle_barrel;
                            double radians = angle_bullet * 3.14159265 / 180;
                            Vector2 bullet_direction = { (float)std::cos(radians),(float)std::sin(radians) };
                            bullet.set_position(pos_barrel_center + bullet_direction * length_barrel); // �����ӵ�λ��

                            // ���ſ�����Ч
                            switch (rand() % 3) {
                            case 0: Mix_PlayChannel(-1, sound_fire_1, 0); break;
                            case 1: Mix_PlayChannel(-1, sound_fire_2, 0); break;
                            case 2: Mix_PlayChannel(-1, sound_fire_3, 0); break;
                            }

                            // �����ȫƴд��ȷ��ѡ����һ������
                            if (player_input == current_word) {
                                // �÷�����
                                score += 10;

                                // �ӳ�һ�����л����ʣ�����ҿ�����������
                                SDL_Delay(100);

                                // ����������
                                player_input.clear();

                                // ѡ����һ������
                                auto it = word_dictionary.begin();
                                std::advance(it, rand() % word_dictionary.size());
                                current_word = it->first;
                                current_definition = it->second;
                            }
                        }
                        // �������������ĸ�������κδ���
                    }
                }
                break;
                // �Ƴ���갴ť�¼�����
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