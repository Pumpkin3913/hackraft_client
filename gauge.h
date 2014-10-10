#ifndef GAUGE_H
#define GAUGE_H

// TODO : switch to numerical display if too large.

#include <string>

class Sdl;
class TextRenderer;
class Sprite;

class Gauge {
	private:
		class Sprite * name;
		class Sprite * full;
		class Sprite * empty;
		unsigned int val;
		unsigned int max;
	public:
		Gauge(
			class Sdl * sdl,
			class TextRenderer * tr,
			std::string name,
			class Sprite * full,
			class Sprite * empty);
		~Gauge();
		void set(unsigned int val, unsigned int max);
		void setAspect(class Sprite * full, class Sprite * empty);
		unsigned int getHeight();
		void draw(class Sdl * sdl, signed int x_shift, signed int y_shift);
};

#endif
