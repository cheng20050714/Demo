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

// ��Ϸ����Ҫ����Դ����
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

// ����Ϸ�߼��йصı�������
int hp = 10;
int score = 0;
std::vector<Bullet> bullet_list;
std::vector<Chicken*> chicken_list;

double num_per_gen = 1.5;
Timer timer_generate;  // ��ʬ�����ɶ�ʱ��
Timer timer_auto_fire; // �Զ�����ʱ��
bool is_barrel_firing = false; // �Ƿ����ڿ���ı�־
float barrel_fire_time = 0;    // ���𶯻��Ѳ���ʱ��
const float BARREL_FIRE_DURATION = 0.3f; // ���𶯻�����ʱ��

Vector2 pos_crosshair;
double angle_barrel = 0; // �ڹ���ת�Ƕ�
const Vector2 pos_battery = { 640,600 }; // ��̨��������λ��
const Vector2 pos_barrel = { 582,585 }; // �ڹ�����תĬ��λ��
const SDL_FPoint center_barrel = { 48,15 }; // �ڹ���ת��������

Animation animation_barrel_fire; // �ڹܿ��𶯻�

// func������������Դ
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
    atlas_boss_1.load(renderer, "../resources/joke_bear/joke_bear_1-%d.png", 11);
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
    font_large = TTF_OpenFont("../resources/IPix.ttf", 40);
}

// func���ͷ�������Դ
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

// func: ��ʼ��������Դ
void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(32);

    window = SDL_CreateWindow(u8"demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_ShowCursor(SDL_DISABLE); // ���������

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

    timer_auto_fire.set_one_shot(false);
    timer_auto_fire.set_wait_time(0.5f);
    timer_auto_fire.set_on_timeout([&]() {
        is_barrel_firing = true;
        barrel_fire_time = 0;
        animation_barrel_fire.reset();

        static const float length_barrel = 105;
        static const Vector2 pos_barrel_center = { 640, 610 };

        bullet_list.emplace_back(angle_barrel);
        Bullet& bullet = bullet_list.back();
        double angle_bullet = angle_barrel;
        double radians = angle_bullet * 3.14159265 / 180;
        Vector2 bullet_direction = { (float)std::cos(radians), (float)std::sin(radians) };
        bullet.set_position(pos_barrel_center + bullet_direction * length_barrel);

        switch (rand() % 3) {
        case 0: Mix_PlayChannel(-1, sound_fire_1, 0); break;
        case 1: Mix_PlayChannel(-1, sound_fire_2, 0); break;
        case 2: Mix_PlayChannel(-1, sound_fire_3, 0); break;
        }
        });

    animation_barrel_fire.add_frame(&atlas_barrel_fire);
    animation_barrel_fire.set_position(pos_battery);
    animation_barrel_fire.set_center(center_barrel);
    animation_barrel_fire.set_interval(0.1f);

    Mix_PlayMusic(music_bgm, -1);
}




// func������
void on_update(float delta) {
    timer_generate.on_update(delta);
    timer_auto_fire.on_update(delta);

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

        // ����ӵ��뼦����ײ
        for (Chicken* chicken : chicken_list) {
            if (!chicken->check_alive()) continue;  // �����Ѿ������ļ�

            // �򵥵���ײ��⣺����ӵ��ͼ���λ���Ƿ��㹻��
            Vector2 bullet_pos = bullet.get_position();
            Vector2 chicken_pos = chicken->get_position();
            Vector2 diff = bullet_pos - chicken_pos;

            // �������С��50���أ����Ը���ʵ���������������Ϊ������ײ
            if (diff.length() < 50.0f) {
                bullet.on_hit();  // ʹ�ӵ�ʧЧ
                chicken->on_hurt();  // ������������Ч��
                Mix_PlayChannel(-1, sound_explosion, 0);  // ���ű�ը��Ч
                score += 100;  // ���ӷ���
                break;  // һ���ӵ�ֻ�ܴ���һֻ��
            }
        }
    }

    // chicken ����
    for (Chicken* chicken : chicken_list) {
        chicken->on_update(delta);
    }

    // ����ʧЧ���ӵ�
    bullet_list.erase(std::remove_if(
        bullet_list.begin(), bullet_list.end(),
        [](const Bullet& bullet) {
            return bullet.can_remove();
        }),
        bullet_list.end());

    // ����ʧЧ�� chicken
    chicken_list.erase(std::remove_if(
        chicken_list.begin(), chicken_list.end(),
        [](Chicken* chicken) {
            if (chicken->can_remove()) {
                delete chicken;  // �ǵ�ɾ��ָ��
                return true;
            }
            return false;
        }),
        chicken_list.end());
}
// func: ��Ⱦ
void on_render(const Camera& camera) {
    // ���Ʊ���ͼ
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

    // �����ӵ�
    for (const Bullet& bullet : bullet_list)
        bullet.on_render(camera);

    // �����δ�������Ⱦ chicken
    for (const Chicken* chicken : chicken_list) {
        chicken->on_render(camera);
    }
    // ������̨
    {
        int width_battery, height_battery;
        SDL_QueryTexture(tex_battery, nullptr, nullptr, &width_battery, &height_battery);
        const SDL_FRect rect_battery = {
            pos_battery.x - width_battery / 2.0f,
            pos_battery.y - height_battery / 2.0f,
            (float)width_battery,(float)height_battery
        };
        camera.render_texture(tex_battery, nullptr, &rect_battery, 0, nullptr);

        // �����ڹ�
        int width_barrel, height_barrel;
        SDL_QueryTexture(tex_barrel_idle, nullptr, nullptr, &width_barrel, &height_barrel);
        const SDL_FRect rect_barrel = {
            pos_barrel.x, pos_barrel.y,
            (float)width_barrel, (float)height_barrel
        };

        if (is_barrel_firing) {
            animation_barrel_fire.set_rotation(angle_barrel);
            animation_barrel_fire.set_position({ pos_battery.x + 60, pos_battery.y + 10 });
            animation_barrel_fire.on_render(camera);
        }
        else {
            camera.render_texture(tex_barrel_idle, nullptr, &rect_barrel, angle_barrel, &center_barrel);
        }
    }
}

// func: ��ѭ��
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
                // �������λ�ü����ڹܽǶ�
                int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                Vector2 target_direction = { (float)mouse_x - pos_battery.x, (float)mouse_y - pos_battery.y };
                angle_barrel = std::atan2(target_direction.y, target_direction.x) * 180 / 3.14159265f;
                break;
            }
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

    return 0;
}
