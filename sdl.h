#ifndef SDL_H
#define SDL_H

class Sprite;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <list>
#include <map>

class Sdl {
private:
	SDL_Window * window;
	SDL_Renderer * renderer;
	const Uint8 * keyboard;
	int max_key;
	unsigned int width;
	unsigned int height;

	std::list<SDL_Scancode> scancodes;
	std::map<std::string, class Sprite *> sprites;

	Sdl(const class Sdl&) = delete;
	class Sdl & operator=(const class Sdl&) = delete;

public:

	Sdl(unsigned int width, unsigned int height);
	~Sdl();
	void error(std::string message);
	void error_ttf(std::string message);
	unsigned int get_width();
	unsigned int get_height();
	int get_max_key();
	SDL_Renderer * get_renderer();
	void set_icon(std::string filename);
	void set_title(std::string title);
	void next_frame();
	bool key(SDL_Scancode key);
	bool keydown(SDL_Scancode key);
	void load_sprite(std::string sprite_name, std::string file_name);
	void load_sprite(std::string sprite_name, class Sprite * sprite);
	class Sprite * get_sprite(std::string name);
	void draw(std::string sprite_name, int x, int y);
	void draw_center(std::string sprite_name, int x, int y);
};

#endif // SDL_H