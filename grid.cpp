#include "grid.h"

#include "sdl.h"
#include "tileset.h"
#include "sprite.h"

Grid::Grid(
	unsigned int width,
	unsigned int height,
	unsigned int tile_width,
	unsigned int tile_height,
	int x_shift,
	int y_shift
) :
	width(width),
	height(height),
	tile_width(tile_width),
	tile_height(tile_height),
	x_shift(x_shift),
	y_shift(y_shift),
	tiles(std::vector<unsigned int>(width*height, 0))
{ }

unsigned int Grid::get_width() {
	return(this->width);
}

unsigned int Grid::get_height() {
	return(this->height);
}

unsigned int Grid::get(unsigned int x, unsigned int y) {
	if(x < this->width && y < this->height) {
		return(this->tiles.at(y * this->width + x));
	} else {
		return(0);
	}
}

class Sprite * Grid::get(unsigned int x, unsigned int y,
		class Tileset * tileset) {
	return(tileset->get(this->get(x,y)));
}

void Grid::set(unsigned int x, unsigned int y, unsigned int val) {
	if(x < this->width && y < this->height) {
		this->tiles.at(y * this->width + x) = val;
	}
}

void Grid::fill(unsigned int val) {
	this->tiles.assign(this->width * this->height, val);
}

void Grid::draw(class Sdl * sdl, class Tileset * tileset) {
	class Sprite * sprite;
	for(unsigned int y=0; y<this->height; y++) {
		for(unsigned int x=0; x<this->width; x++) {
			sprite = this->get(x, y, tileset);
			sprite->draw(sdl,
				x*this->tile_width + this->x_shift,
				y*this->tile_height + this->y_shift);
		}
	}
}

