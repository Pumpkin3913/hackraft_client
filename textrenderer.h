#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

class Sdl;
class Sprite;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class TextRenderer {
	private:
		// unsigned int height;
		TTF_Font * font;
		SDL_Color color;
	public:
		TextRenderer(
			class Sdl * sdl,
			std::string font_filename,
			unsigned int height,
			unsigned char red = 255,
			unsigned char green = 255,
			unsigned char blue = 255,
			unsigned char alpha = 255
		);
		~TextRenderer();
		class Sprite * render(class Sdl * sdl, std::string message);
		unsigned int lineskip();
};

#endif //TEXTRENDERER_H
