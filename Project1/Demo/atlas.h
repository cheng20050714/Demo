#ifndef _ATLAS_H_  
#define _ATLAS_H_  

#include<SDL.h>  
#include<SDL_image.h>  

#include<vector>  
#include<string>  
#include<iostream> 


class Atlas {  
public:  
Atlas() = default;  
~Atlas() {  
	for (SDL_Texture* texture : tex_list) {  
		SDL_DestroyTexture(texture);  
	}  
}  

void load(SDL_Renderer* renderer, const char* path_template, int num) {  
	for (int i = 0; i < num; i++) {  
		char path_file[256];  
		sprintf_s(path_file, path_template, i + 1);  
		SDL_Texture* texture = IMG_LoadTexture(renderer, path_file); //图片文件直接加载为纹理  
		tex_list.push_back(texture);  
	}  
}  

void clear() {  
	tex_list.clear();  
	rect_list.clear();  
}  

int get_size() {  
	return (int)tex_list.size();  
}  

SDL_Texture* get_texture(int idx) {  
	if (idx < 0 || idx >= tex_list.size()) {  
		return nullptr;  
	}  
	return tex_list[idx];  
}  

void add_texture(SDL_Texture* texture) {  
	tex_list.push_back(texture);  
}  

void load_from_spritesheet(SDL_Renderer* renderer, const char* path, int frame_count) {  
	SDL_Texture* texture = IMG_LoadTexture(renderer, path);  
	if (!texture) {  
		std::cerr << "Failed to load texture: " << path << std::endl;  
		return;  
	}  

	int width, height;  
	SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);  

	int frame_width = width / frame_count;  

	for (int i = 0; i < frame_count; ++i) {  
		SDL_Rect rect_src;  
		rect_src.x = i * frame_width;  
		rect_src.y = 0;  
		rect_src.w = frame_width;  
		rect_src.h = height;  

		tex_list.push_back(texture);  
		rect_list.push_back(rect_src);  
	}  
}  

private:  
std::vector<SDL_Texture*> tex_list;  
std::vector<SDL_Rect> rect_list; // 添加 rect_list 成员变量  
};  

#endif // !_ATLAS_H_
