#ifndef CONSOLE_H
#define CONSOLE_H

class Sdl;
class TextRenderer;
class Sprite;

#include <list>
#include <string>

class Console {
	private:
		unsigned int max_line;
		unsigned int width;
		unsigned int height;
		signed int x_shift;
		signed int y_shift;
		unsigned int line_height;
		std::list<class Sprite *> content;

		void add(class Sprite * line);
	public:
		Console(
			class TextRenderer * tr,
			unsigned int width,
			unsigned int height,
			signed int x_shift = 0,
			signed int y_shift = 0);
		~Console();
		void add_line(class Sdl * sdl, class TextRenderer * tr, std::string text);
		void draw(class Sdl * sdl);
};

// TODO : scrolling.
// TODO : auto cleaning.

#endif // CONSOLE_H
