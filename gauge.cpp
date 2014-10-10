#include "gauge.h"

#include "sdl.h"
#include "textrenderer.h"
#include "sprite.h"

Gauge::Gauge(
	class Sdl * sdl,
	class TextRenderer * tr,
	std::string name,
	class Sprite * full,
	class Sprite * empty
) :
	name(tr->render(sdl, name)),
	full(full),
	empty(empty),
	val(0),
	max(0)
{ }

Gauge::~Gauge() {
	delete(this->name);
}

void Gauge::set(unsigned int val, unsigned int max) {
	this->val = val;
	this->max = max;
}

void Gauge::setAspect(class Sprite * full, class Sprite * empty) {
	this->full = full;
	this->empty = empty;
}

unsigned int Gauge::getHeight() {
	unsigned int name_h = this->name->get_height();
	unsigned int full_h = this->full->get_height();
	unsigned int empty_h = this->empty->get_height();
	unsigned int max = name_h;
	if(max < full_h) max = full_h;
	if(max < empty_h) max = empty_h;
	return(max);
}

void Gauge::draw(class Sdl * sdl, signed int x_shift, signed int y_shift) {
	unsigned int x = 0;
	unsigned int i = 0;
	this->name->draw(sdl, x + x_shift, y_shift);
	x += this->name->get_width();
	for(i=0; i<val; i++) {
		this->full->draw(sdl, x + x_shift, y_shift);
		x += this->full->get_width();
	}
	for(; i<max; i++) {
		this->empty->draw(sdl, x + x_shift, y_shift);
		x += this->empty->get_width();
	}
}

