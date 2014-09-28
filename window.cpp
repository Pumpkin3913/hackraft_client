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
	y_center(y_center)
{
}

void Window::set_center(int x_center, int y_center) {
	this->x_center = x_center;
	this->y_center = y_center;
}

void Window::draw(class Sdl * sdl, class Grid * grid, class Tileset * tileset) {
	class Sprite * sprite;

	int top = this->y_center - this->height/2;
	int left = this->x_center - this->width/2;
	int bot = top + this->height;
	int right = left + this->width;

	for(int y = top ; y < bot; y++) {
		for(int x = left; x < right; x++) {
			if(x>=0 && x<grid->get_width() && y>=0 && y<grid->get_height()) {
				sprite = grid->get(x, y, tileset);
				sprite->draw(sdl,
					(x-left)*this->tile_width + this->x_shift,
					(y-top)*this->tile_height + this->y_shift);
			}
		}
	}
}

