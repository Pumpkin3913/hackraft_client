#include "sprite.h"

#include "sdl.h"

#include <SDL2/SDL_image.h>

Sprite::Sprite(class Sdl * sdl, const std::string filename) {
	SDL_Surface * surface = IMG_Load(filename.c_str());
	if(surface == NULL)
		sdl->error(filename + ": unable to load file");
	this->texture = SDL_CreateTextureFromSurface(sdl->get_renderer(), surface);
	if(texture == NULL)
		sdl->error(filename + ": unable to create texture from surface");
	this->w = surface->w;
	this->h = surface->h;
	SDL_FreeSurface(surface);
}

Sprite::Sprite(class Sdl * sdl, SDL_Surface * surface) {
	this->texture = SDL_CreateTextureFromSurface(sdl->get_renderer(), surface);
	if(texture == NULL)
		sdl->error("Unable to create texture from surface");
	this->w = surface->w;
	this->h = surface->h;
}

Sprite::~Sprite() {
	SDL_DestroyTexture(this->texture);
}

unsigned int Sprite::get_width() {
	return(this->w);
}

unsigned int Sprite::get_height() {
	return(this->h);
}

void Sprite::draw(class Sdl * sdl, int x, int y) {
	SDL_Rect rect;
	rect.w = this->w;
	rect.h = this->h;
	rect.x = x;
	rect.y = y;
	SDL_RenderCopy(sdl->get_renderer(), this->texture, NULL, &rect);
}

void Sprite::draw_center(class Sdl * sdl, float x, float y) {
	this->draw(sdl, x - this->w/2, y - this->h/2);
}

