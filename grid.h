#ifndef GRID_H
#define GRID_H

class Sdl;
class Tileset;
class Sprite;

#include <vector>

class Grid {
	private:
		unsigned int width;  // tiles
		unsigned int height; // tiles
		unsigned int tile_width;  // px
		unsigned int tile_height; // px
		int x_shift; // px
		int y_shift; // px
		std::vector<unsigned int> tiles;

	public:
		Grid(
			unsigned int width,
			unsigned int height,
			unsigned int tile_width,
			unsigned int tile_height,
			int x_shift = 0,
			int y_shift = 0
		);
		unsigned int get_width();
		unsigned int get_height();
		unsigned int get(unsigned int x, unsigned int y);
		class Sprite * get(unsigned int x, unsigned int y,
				class Tileset * tileset);
		void set(unsigned int x, unsigned int y, unsigned int val);
		void fill(unsigned int val);
		void draw(class Sdl * sdl, class Tileset * tileset);
};

#endif
