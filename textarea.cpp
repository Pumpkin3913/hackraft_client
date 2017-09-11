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
	tr(tr),
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

void TextArea::add_string(class Sdl * sdl, std::string string) {
	auto* rendered = this->tr->render(sdl, string);
	if(rendered == nullptr) return;
	this->content.push_back(rendered);
	this->text += string;
}

void TextArea::pop_char(class Sdl * sdl) {
	unsigned int n = this->text.size();
	char c = this->text.back();
	while(n > 0 and (c & 0xC0) == 0x80) {
		// "(c & 0xC0) != 0x80" is a hack to detect leading/trailing UTF-8 characters.
		this->text.pop_back();
		n--;
		c = this->text.back();
	}

	if(n > 0) {
		this->text.pop_back();
	}

	// Refresh graphical content.
	for(auto* s : this->content) { delete(s); }
	this->content.clear();
	auto* sprite = this->tr->render(sdl, this->text);
	if(sprite == nullptr) { return; }
	this->content.push_back(sprite);
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

