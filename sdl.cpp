#include "sdl.h"

#include "sprite.h"

#include <iostream>
#include <cstdlib>
#include <SDL2/SDL_image.h>

Sdl::Sdl(
	unsigned int width,
	unsigned int height,
	Uint32 frame_duration
) :
	window(NULL),
	renderer(NULL),
	keyboard(NULL),
	max_key(0),
	width(width),
	height(height),
	quit(false),
	frame_duration(frame_duration),
	last_frame_ticks(0),
	scancodes(),
	sprites()
{
	// Initialize SDL video.
	if(SDL_Init(SDL_INIT_VIDEO /*|SDL_INIT_AUDIO*/) < 0) {
		this->error("Unable to init SDL");
	}

	// Initialize TTF.
	if(TTF_Init()) {
		this->error_ttf("Unable to init TTF");
	}

	// Initialize IMG.
	if(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
		this->error_img("Unable to init IMG");
	}

	// Create window.
	window = SDL_CreateWindow(
			NULL,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			0, 0, 0);
	if(!window)
		this->error("Unable to open new window");

	// Create renderer.
	renderer = SDL_CreateRenderer(window, -1, 0);
	if(!renderer)
		this->error("Unable to create new renderer");

	// Configure window.
	SDL_RenderSetLogicalSize(renderer, this->width, this->height);
	SDL_SetWindowSize(window, this->width, this->height);
	SDL_SetWindowPosition(window,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	// SDL_ShowCursor(SDL_DISABLE);
	// SDL_SetWindowFullscreen(this->window, SDL_WINDOW_FULLSCREEN);
	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	// makes the scaled rendering look smoother. // DON'T DO THAT!!!

	// Store keyboard address.
	this->keyboard = SDL_GetKeyboardState(&(this->max_key));

	// First clear and present.
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
	SDL_RenderClear(this->renderer);
	SDL_RenderPresent(this->renderer);
}

Sdl::~Sdl() {
	for(auto it : this->sprites) {
		delete(it.second);
	}

	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Sdl::error(std::string message) {
	std::cerr << message << " : " << SDL_GetError() << std::endl;
	exit(1);
}

void Sdl::error_ttf(std::string message) {
	std::cerr << message << " : " << TTF_GetError() << std::endl;
	exit(1);
}

void Sdl::error_img(std::string message) {
	std::cerr << message << " : " << IMG_GetError() << std::endl;
	exit(1);
}

unsigned int Sdl::get_width() {
	return(this->width);
}

unsigned int Sdl::get_height() {
	return(this->height);
}

int Sdl::get_max_key() {
	return(this->max_key);
}

SDL_Renderer * Sdl::get_renderer() {
	return(this->renderer);
}

int Sdl::get_mouse_x() {
	return(this->mouse_x);
}

int Sdl::get_mouse_y() {
	return(this->mouse_y);
}

void Sdl::set_icon(std::string filename) {
	SDL_Surface* icon = IMG_Load(filename.c_str());
	if(!icon)
		this->error("Unable to load icon file " + filename);
	SDL_SetWindowIcon(this->window, icon);
	SDL_FreeSurface(icon);
}

void Sdl::set_title(std::string title) {
	SDL_SetWindowTitle(this->window, title.c_str());
}

void Sdl::next_frame() {
	SDL_Event event;

	SDL_RenderPresent(this->renderer);
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
	SDL_RenderClear(this->renderer);

	Uint32 that_frame_duration = SDL_GetTicks() - this->last_frame_ticks;
	if(that_frame_duration < this->frame_duration) {
		SDL_Delay(this->frame_duration - that_frame_duration);
	}
	this->last_frame_ticks = SDL_GetTicks();

	this->scancodes.clear();
	this->clics.clear();
	this->wheels.clear();
	this->text_input = "";
	// this->quit = false;

	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_KEYDOWN) {
			this->scancodes.push_back(event.key.keysym.scancode);
		} else if(event.type == SDL_TEXTINPUT) {
			this->text_input += event.text.text;
		} else if(event.type == SDL_MOUSEBUTTONDOWN) {
			this->clics.push_back(
				Clic(event.button.x, event.button.y, event.button.button));
		} else if(event.type == SDL_MOUSEWHEEL) {
			int x = event.wheel.x;
			int y = event.wheel.y;
			if(event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
				x *= -1;
				y *= -1;
			}
			this->wheels.push_back(Wheel{x, y});
		} else if(event.type == SDL_QUIT) {
			this->quit = true;
		}
	}

	SDL_GetMouseState(&this->mouse_x, &this->mouse_y);
}

bool Sdl::key(SDL_Scancode key) {
	if(key < this->max_key) {
		return(this->keyboard[key] == 1);
	} else {
		return(false);
	}
}

bool Sdl::keydown(SDL_Scancode key) {
	for(SDL_Scancode it : this->scancodes) {
		if(it == key) {
			return(true);
		}
	}
	return(false);
}

bool Sdl::any_keydown() {
	return(not this->scancodes.empty());
}

std::string Sdl::get_text() {
	return(this->text_input);
}

std::list<struct Clic> Sdl::get_clics() {
	return(this->clics);
}

std::list<struct Wheel> Sdl::get_wheels() {
	return(this->wheels);
}

bool Sdl::has_quit() {
	return(this->quit);
}

void Sdl::load_sprite(std::string sprite_name, std::string file_name) {
	this->load_sprite(sprite_name, new Sprite(this, file_name));
}

void Sdl::load_sprite(std::string sprite_name, class Sprite * sprite) {
	if(this->sprites[sprite_name] != NULL) {
		delete(this->sprites[sprite_name]);
	}
	this->sprites[sprite_name] = sprite;
}

class Sprite * Sdl::get_sprite(std::string name) {
	class Sprite * sprite;
	sprite = this->sprites[name];
	if(sprite == NULL) {
		this->load_sprite(name, "data\\"+name+".bmp");
		sprite = this->sprites[name];
		if(sprite == NULL) {
			std::cerr << "Sprite '" << name << "' doesn't exist." << std::endl;
			exit(1);
		}
	}
	return(sprite);
}

void Sdl::draw(std::string sprite_name, int x, int y) {
	Sprite * sprite = this->get_sprite(sprite_name);
	if(sprite) {
		sprite->draw(this, x, y);
	}
}

void Sdl::draw_center(std::string sprite_name, int x, int y) {
	Sprite * sprite = this->get_sprite(sprite_name);
	if(sprite) {
		sprite->draw_center(this, x, y);
	}
}

