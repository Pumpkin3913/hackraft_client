#ifndef TEXTAREA_H
#define TEXTAREA_H

class Sdl;
class TextRenderer;
class Sprite;

#include <vector>
#include <string>

class TextArea {
	private:
		unsigned int width;
		unsigned int height;
		signed int x_shift;
		signed int y_shift;
		unsigned int line_height;
		std::vector<class Sprite *> content;
		std::string text;
	public:
		TextArea(
			class TextRenderer * tr,
			unsigned int width,
			unsigned int height,
			signed int x_shift = 0,
			signed int y_shift = 0);
		~TextArea();
		void add_string(class Sdl * sdl, class TextRenderer * tr, std::string string);
		void pop_char();
		std::string get_text();
		void clear();
		void draw(class Sdl * sdl);
};

#endif // TEXTAREA_H
