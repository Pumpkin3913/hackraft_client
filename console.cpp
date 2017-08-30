#include "console.h"

#include "sdl.h"
#include "textrenderer.h"
#include "sprite.h"

#include <vector>

Console::Console(
	class TextRenderer * tr,
	unsigned int width,
	unsigned int height,
	signed int x_shift,
	signed int y_shift
) :
	width(width),
	height(height),
	x_shift(x_shift),
	y_shift(y_shift)
{
	this->line_height = tr->lineskip();
	this->max_line = this->height / this->line_height;
}

Console::~Console() {
	for(class Sprite * line : this->content) {
		delete(line);
	}
}

void Console::add(class Sprite * line) {
	if(line == nullptr) return;
	if(this->content.size() >= this->max_line) {
		this->content.pop_back();
	}
	this->content.push_front(line);
}

void Console::add_line(class Sdl * sdl, class TextRenderer * tr, std::string text) {
	if(text != "") {
		// TODO : line split
		this->add(tr->render(sdl, text));
	}
}

void Console::draw(class Sdl * sdl) {
	int line_no = 0;
	for(class Sprite * line : this->content) {
		line->draw(sdl,
			this->x_shift,
			this->y_shift + this->height - this->line_height*(line_no+1));
		line_no++;
	}
}

