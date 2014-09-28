#ifndef TILESET_H
#define TILESET_H

class Sdl;
class Sprite;

#include <vector>
#include <string>

class Tileset {
	private:
		std::vector<class Sprite *> tiles;

	public:
		Tileset(
			class Sdl * sdl,
			std::string filename,
			unsigned int tile_width, // px
			unsigned int tile_height // px
		);
		~Tileset();
		unsigned int size();
		class Sprite * get(unsigned int index);
};

#endif
