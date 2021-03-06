#include "tileset.h"

#include "sdl.h"
#include "sprite.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Tileset::Tileset(
	unsigned int tile_width,
	unsigned int tile_height
) :
	tile_width(tile_width),
	tile_height(tile_height),
	tiles(std::vector<class Sprite *>())
{ }

Tileset::Tileset(
	class Sdl * sdl,
	std::string filename,
	unsigned int tile_width,
	unsigned int tile_height
) :
	tile_width(tile_width),
	tile_height(tile_height),
	tiles(std::vector<class Sprite *>())
{
	this->append(sdl, filename);
}

Tileset::~Tileset() {
	for(class Sprite * sprite : this->tiles) {
		delete(sprite);
	}
}

unsigned int Tileset::get_tile_width() {
	return(this->tile_width);
}

unsigned int Tileset::get_tile_height() {
	return(this->tile_height);
}

unsigned int Tileset::size() {
	return(this->tiles.size());
}

class Sprite * Tileset::get(unsigned int index) {
	class Sprite * toret;
	if(index >= this->tiles.size()) {
		toret = this->tiles.at(0);
	} else {
		toret = this->tiles.at(index);
	}
	return(toret);
}

void Tileset::append(class Sdl * sdl, std::string filename) {
	SDL_Surface * surface_source;
	SDL_Surface * surface_buffer;
	SDL_Rect src;

	surface_source = IMG_Load(filename.c_str());
	if(surface_source == NULL)
		sdl->error(filename + ": unable to load file");

	src.w = tile_width;
	src.h = tile_height;

	// from SDL wiki
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	for(int y_shift=0; (y_shift+1)*src.h <= surface_source->h; y_shift++) {
		for(int x_shift=0; (x_shift+1)*src.w <= surface_source->w; x_shift++) {
			src.x = x_shift * src.w;
			src.y = y_shift * src.h;

			surface_buffer = SDL_CreateRGBSurface(
					0, src.w, src.h, 32,
					rmask, gmask, bmask, amask);
			if(surface_buffer == NULL)
				sdl->error("Unable to create RGB surface");

			SDL_BlitSurface(surface_source, &src, surface_buffer, NULL);
			this->tiles.push_back(new Sprite(sdl, surface_buffer));
			SDL_FreeSurface(surface_buffer);
		}
	}
	SDL_FreeSurface(surface_source);
}

