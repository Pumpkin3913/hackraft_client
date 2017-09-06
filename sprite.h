#ifndef SPRITE_H
#define SPRITE_H

class Sdl;

#include <SDL2/SDL.h>
#include <string>

class Sprite {
	private:
		SDL_Texture * texture;
		unsigned int w;
		unsigned int h;

	public:
		Sprite(class Sdl * sdl, const std::string filename);
		Sprite(class Sdl * sdl, SDL_Surface * surface);
		~Sprite();
		unsigned int get_width();
		unsigned int get_height();
		void draw(class Sdl * sdl, int x, int y, float zoom = 1.0);
		void draw_center(class Sdl * sdl, float x, float y);
};

#endif // SPRITE_H
