#include "textarea.h"

#include "sdl.h"
#include "textrenderer.h"
#include "sprite.h"

TextArea::TextArea(
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
}

TextArea::~TextArea() {
	for(class Sprite * chara : this->content) {
		delete(chara);
	}
}

void TextArea::add_char(class Sdl * sdl, class TextRenderer * tr, char c) {
	this->content.push_back(tr->render(sdl, std::string()+c));
	this->text.push_back(c);
}

void TextArea::pop_char() {
	if(this->content.size() > 0) {
		this->content.pop_back();
		this->text.pop_back();
	}
}

std::string TextArea::get_text() {
	return(this->text);
}

void TextArea::clear() {
	this->content.clear();
	this->text.clear();
}

void TextArea::draw(class Sdl * sdl) {
	int total_width = 0;
	std::vector<int> newline_indice;
	newline_indice.push_back(0); // First char is first line's first char.
	for(int no_char = 0; no_char < this->content.size(); no_char++) {
		total_width += this->content.at(no_char)->get_width();
		if(total_width > this->width) {
			newline_indice.push_back(no_char);
			total_width = this->content.at(no_char)->get_width();
		}
	}
	int last_char = this->content.size();
	int line_shift = 0;
	for(int i = newline_indice.size()-1; i >= 0; i--) {
		int first_char = newline_indice[i];
		total_width = 0;
		for(int no_char = first_char; no_char < last_char; no_char++) {
			this->content.at(no_char)->draw(sdl,
					this->x_shift + total_width,
					this->y_shift + this->height - this->line_height*(line_shift+1));
			total_width += this->content.at(no_char)->get_width();
		}
		last_char = first_char;
		line_shift++;
	}
}

