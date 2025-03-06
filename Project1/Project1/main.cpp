#define SDL_MAIN_HANDLED

#include<iostream>
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_mixer.h>


int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG | IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3); //init the thirdpartys

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); //fixed code to init the sound track

	SDL_Window* window = SDL_CreateWindow(u8"hello demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* suf_img = IMG_Load("..\\sources\\chicken_fast_1.png"); //内存结构体
	SDL_Texture* tex_img = SDL_CreateTextureFromSurface(renderer, suf_img); //固定步骤


	bool is_quit = false;

	SDL_Event event;
	SDL_Point pos_cursor = { 0,0 };
	SDL_Rect rect_img;

	rect_img.w = suf_img->w;
	rect_img.h = suf_img->h;


	while (!is_quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_quit = true;
			}
			else if (event.type == SDL_MOUSEMOTION) {
				pos_cursor.x = event.motion.x;
				pos_cursor.y = event.motion.y;
			}
		}


		//处理数据
		rect_img.x = pos_cursor.x;
		rect_img.y = pos_cursor.y;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		//渲染绘图
		SDL_RenderCopy(renderer, tex_img, nullptr, &rect_img);

		SDL_RenderPresent(renderer);

	}

	return 0;
}