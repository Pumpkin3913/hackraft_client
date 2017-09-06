#include "window.h"

#include "sdl.h"
#include "grid.h"
#include "tileset.h"
#include "sprite.h"

Window::Window(
	unsigned int width,
	unsigned int height,
	unsigned int tile_width,
	unsigned int tile_height,
	int x_shift,
	int y_shift,
	int x_center,
	int y_center
) :
	width(width),
	height(height),
	tile_width(tile_width),
	tile_height(tile_height),
	x_shift(x_shift),
	y_shift(y_shift),
	x_center(x_center),
	y_center(y_center),
	zoom(1.0)
{
}

void Window::set_center(int x_center, int y_center) {
	this->x_center = x_center;
	this->y_center = y_center;
}

void Window::draw(class Sdl * sdl, class Grid * grid, class Tileset * ts) {
	class Sprite * sprite;

	int top = this->y_center - this->height/2 / zoom;
	int left = this->x_center - this->width/2 / zoom;
	int bot = top + this->height / zoom;
	int right = left + this->width / zoom;

	for(int y = top ; y < bot; y++) {
		for(int x = left; x < right; x++) {
			if(x>=0 && x<grid->get_width() && y>=0 && y<grid->get_height()) {
				sprite = grid->get(x, y, ts);
				this->draw(sdl, sprite, x, y);
			}
		}
	}
}

void Window::draw(class Sdl * sdl, class Sprite * sprite, int x, int y) {
	int top = this->y_center - this->height/2 / zoom;
	int left = this->x_center - this->width/2 / zoom;
	sprite->draw(sdl,
		(x-left)*this->tile_width*zoom + this->x_shift,
		(y-top)*this->tile_height*zoom + this->y_shift,
		zoom);
}

void Window::draw(class Sdl * sdl, int id, class Tileset * ts, int x, int y) {
	this->draw(sdl, ts->get(id), x, y);
}

void Window::set_zoom(float zoom) {
	this->zoom = zoom;
}
