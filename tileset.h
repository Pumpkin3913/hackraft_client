#ifndef TILESET_H
#define TILESET_H

class Sdl;
class Sprite;

#include <vector>
#include <string>

class Tileset {
	private:
		unsigned int tile_width; // px
		unsigned int tile_height; // px
		std::vector<class Sprite *> tiles;

	public:
		Tileset(
			unsigned int tile_width,
			unsigned int tile_height
		);
		Tileset(
			class Sdl * sdl,
			std::string filename,
			unsigned int tile_width,
			unsigned int tile_height
		);
		~Tileset();
		unsigned int get_tile_width();
		unsigned int get_tile_height();
		unsigned int size();
		class Sprite * get(unsigned int index);
		void append(class Sdl * sdl, std::string filename);
};

#endif
