#ifndef WINDOW_H
#define WINDOW_H

class Sdl;
class Grid;
class Tileset;

class Window {
	private:
		unsigned int width;  // tiles
		unsigned int height; // tiles
		unsigned int tile_width;  // px
		unsigned int tile_height; // px
		int x_shift; // px
		int y_shift; // px
		int x_center; // tile
		int y_center; // tile
	public:
		Window(
			unsigned int width, // tiles
			unsigned int height, // tiles
			unsigned int tile_width, // px
			unsigned int tile_height, // px
			int x_shift = 0, // px
			int y_shift = 0, // px
			int x_center = 0, // tiles
			int y_center = 0 // tiles
		);
		void set_center(int x_center, int y_center);
		void draw(class Sdl * sdl, class Grid * grid, class Tileset * tileset);
};

#endif
