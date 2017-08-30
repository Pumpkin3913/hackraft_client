#include "textrenderer.h"

#include "sdl.h"
#include "sprite.h"

TextRenderer::TextRenderer(
	class Sdl * sdl,
	std::string font_filename,
	unsigned int height,
	unsigned char red,
	unsigned char green,
	unsigned char blue,
	unsigned char alpha
) :
	// height(height),
	font(TTF_OpenFont(font_filename.c_str(), height))
{
	if(!this->font) sdl->error_ttf("Unable to open font");
	this->color.r = red;
	this->color.g = green;
	this->color.b = blue;
	this->color.a = alpha;
}

TextRenderer::~TextRenderer() {
	TTF_CloseFont(this->font);
}

class Sprite * TextRenderer::render(class Sdl * sdl, std::string message) {
    SDL_Surface * surface;
	class Sprite * sprite;

	surface = TTF_RenderUTF8_Blended(this->font, message.c_str(), this->color);
	// if(!surface) sdl->error_ttf("Unable to render text");
	if(!surface) return(nullptr);
	sprite = new Sprite(sdl, surface);
	SDL_FreeSurface(surface);
	return(sprite);
}

unsigned int TextRenderer::lineskip() {
	return(TTF_FontLineSkip(this->font));
}

