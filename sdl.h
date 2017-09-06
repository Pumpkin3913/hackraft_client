#ifndef SDL_H
#define SDL_H

class Sprite;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <list>
#include <map>

struct Clic {
	// Relative to SDL_Window.
	int x;
	int y;
	char button;
	Clic(int x, int y, char button) :
		x(x), y(y), button(button) { }
};

struct Wheel {
	// y<0 is scroll down (backward); y>0 is scroll up (forward).
	int x;
	int y;
};

class Sdl {
private:
	SDL_Window * window;
	SDL_Renderer * renderer;
	const Uint8 * keyboard;
	int max_key;
	unsigned int width;
	unsigned int height;
	bool quit;

	Uint32 frame_duration;
	Uint32 last_frame_ticks;

	std::list<SDL_Scancode> scancodes;
	std::string text_input;
	std::list<struct Clic> clics;
	std::list<struct Wheel> wheels;
	int mouse_x;
	int mouse_y;
	std::map<std::string, class Sprite *> sprites;

	Sdl(const class Sdl&) = delete;
	class Sdl & operator=(const class Sdl&) = delete;

public:

	Sdl(
		unsigned int width,
		unsigned int height,
		Uint32 frame_duration = 1000 / 60
	);
	~Sdl();
	void error(std::string message);
	void error_ttf(std::string message);
	void error_img(std::string message);
	unsigned int get_width();
	unsigned int get_height();
	int get_max_key();
	SDL_Renderer * get_renderer();
	int get_mouse_x();
	int get_mouse_y();
	void set_icon(std::string filename);
	void set_title(std::string title);
	void next_frame();
	bool key(SDL_Scancode key);
	bool keydown(SDL_Scancode key);
	bool any_keydown();
	std::string get_text();
	std::list<struct Clic> get_clics();
	std::list<struct Wheel> get_wheels();
	bool has_quit();
	void load_sprite(std::string sprite_name, std::string file_name);
	void load_sprite(std::string sprite_name, class Sprite * sprite);
	class Sprite * get_sprite(std::string name);
	void draw(std::string sprite_name, int x, int y);
	void draw_center(std::string sprite_name, int x, int y);
};

#endif // SDL_H
