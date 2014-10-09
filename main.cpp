#include "sdl.h"
#include "luaconfig.h"
#include "tileset.h"
#include "grid.h"
#include "window.h"
#include "socket.h"
#include "textrenderer.h"
#include "console.h"
#include "textarea.h"

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <map>
#include <list>

struct Player {
	int id;
	int aspect;
	int x;
	int y;
	Player(int id, int aspect, int x, int y) :
		id(id), aspect(aspect), x(x), y(y) { }
};

std::vector<std::string> split(std::string input, char delim) {
	std::vector<std::string> output;
	int begin = 0;
	int end = 0;
	while((end = input.find(delim, begin)) != std::string::npos) {
		output.push_back(input.substr(begin, end - begin));
		begin = end+1;
	}
	output.push_back(input.substr(begin));
	return(output);
}

int main() {
	bool stop = false;
	int follow_id = 0;
	std::string input;
	std::vector<std::string> tokens;
	std::map<int, struct Player> players;
	class LuaConfig * conf = new LuaConfig("conf.lua");
	class Sdl * sdl = new Sdl(
			conf->get_int("screen_width"),
			conf->get_int("screen_height"));
	class Tileset * tileset = new Tileset(sdl,
			std::string(conf->get_string("tileset_filename")),
			conf->get_int("tileset_width"),
			conf->get_int("tileset_height"));
	class TextRenderer * font = new TextRenderer(
			sdl,
			conf->get_string("font_filename"),
			conf->get_int("font_height"),
			conf->get_int("font_red"),
			conf->get_int("font_green"),
			conf->get_int("font_blue"));
	class Console * console = new Console(
			font,
			conf->get_int("screen_width"),
			conf->get_int("console_height")*font->lineskip(),
			0, // x_shift
			conf->get_int("screen_height") // y_shift
				- conf->get_int("console_height")*font->lineskip()
				- conf->get_int("textarea_height")*font->lineskip()
			);
	class TextArea * textarea = new TextArea(
			font,
			conf->get_int("screen_width"),
			conf->get_int("textarea_height")*font->lineskip(),
			0, // x_shift
			conf->get_int("screen_height") // y_shift
				- conf->get_int("textarea_height")*font->lineskip()
			);
	class Window * window = new Window(
			conf->get_int("screen_width") / conf->get_int("tileset_width"),
			(conf->get_int("screen_height")
				- conf->get_int("console_height")*font->lineskip()
				- conf->get_int("textarea_height")*font->lineskip())
					/ conf->get_int("tileset_height"),
			conf->get_int("tileset_width"),
			conf->get_int("tileset_height"));
	class Socket * socket = new Socket(
			conf->get_int("port"),
			conf->get_string("address"));
	socket->setNonBlock();
	class Grid * grid = NULL;
	int tileset_width = conf->get_int("tileset_width");
	int tileset_height = conf->get_int("tileset_height");

	if(conf->get_string("icon") != "") {
		sdl->set_icon(conf->get_string("icon"));
	}
	if(conf->get_string("title") != "") {
		sdl->set_title(conf->get_string("title"));
	}

	// delete(conf); // FIXME: It cause a double free error and I don't know why.

	while(not stop) {

		// Draw.

		console->draw(sdl);
		textarea->draw(sdl);
		if(grid) {
			window->draw(sdl, grid, tileset);
			if(not sdl->key(SDL_SCANCODE_LCTRL)) {
				for(std::pair<int, struct Player> it : players) {
					window->draw(sdl, it.second.aspect, tileset, it.second.x, it.second.y);
				}
			}
		}
		sdl->next_frame();

		// Process Keyboard Input.

		for(char c : sdl->get_text()) {
			textarea->add_char(sdl, font, c);
		}
		if(sdl->keydown(SDL_SCANCODE_UP)) {
			if(sdl->key(SDL_SCANCODE_LSHIFT)) {
				textarea->add_string(sdl, font, " north");
			} else {
				socket->send("move north\n");
			}
		}
		if(sdl->keydown(SDL_SCANCODE_DOWN)) {
			if(sdl->key(SDL_SCANCODE_LSHIFT)) {
				textarea->add_string(sdl, font, " south");
			} else {
				socket->send("move south\n");
			}
		}
		if(sdl->keydown(SDL_SCANCODE_LEFT)) {
			if(sdl->key(SDL_SCANCODE_LSHIFT)) {
				textarea->add_string(sdl, font, " west");
			} else {
				socket->send("move west\n");
			}
		}
		if(sdl->keydown(SDL_SCANCODE_RIGHT)) {
			if(sdl->key(SDL_SCANCODE_LSHIFT)) {
				textarea->add_string(sdl, font, " east");
			} else {
				socket->send("move east\n");
			}
		}
		if(sdl->keydown(SDL_SCANCODE_BACKSPACE)) {
			textarea->pop_char();
		}
		if(sdl->keydown(SDL_SCANCODE_RETURN)) {
			std::string input = textarea->get_text();
			if(input.size() > 0) {
				if(input[0] == '/') {
					socket->send(input+'\n');
				} else {
					socket->send("say "+input+'\n');
				}
				textarea->clear();
			}
		}
		if(sdl->keydown(SDL_SCANCODE_ESCAPE)) {
			socket->send("quit\n");
		}

		// Process Server Input.

		input = socket->getline();
		if(input == "") {
			tokens.clear();
		} else {
			tokens = split(input, ' ');
		}
		if(tokens.size() >= 1) {
			if(tokens[0] == "move") {
				if(tokens.size() >= 5) {
					int id = std::stoi(tokens[1]);
					int aspect = std::stoi(tokens[2]);
					int x = std::stoi(tokens[3]);
					int y = std::stoi(tokens[4]);
					try {
						players.at(id).id = id;
						players.at(id).aspect = aspect;
						players.at(id).x = x;
						players.at(id).y = y;
					} catch(...) {
						players.insert(std::pair<int, struct Player>(id, Player(id, aspect, x, y)));
					}
					if(id == follow_id) {
						window->set_center(x, y);
					}
				}
			} else if(tokens[0] == "exit") {
				if(tokens.size() >= 2) {
					int id = std::stoi(tokens[1]);
					players.erase(id);
				}
			} else if(tokens[0] == "follow") {
				if(tokens.size() >= 2) {
					follow_id = std::stoi(tokens[1]);
					try {
						int x = players.at(follow_id).x;
						int y = players.at(follow_id).y;
						window->set_center(x, y);
					} catch(...) { }
				}
			} else if(tokens[0] == "floor") {
				if(tokens.size() >= 4) {
					int w, h;
					std::string name;
					w = std::stoi(tokens[1]);
					h = std::stoi(tokens[2]);
					name = tokens[3];
					players.clear();

					if(grid) delete(grid);
					grid = new Grid(w, h, tileset_width, tileset_height);

					// TODO : waith
					input = socket->getline();
					tokens = split(input, ',');
					if(tokens.size() == w*h) {
						for(int x=0; x<w; x++) {
							for(int y=0; y<h; y++) {
								grid->set(x, y, std::stoi(tokens.at(y*w+x)));
							}
						}
					}
				}
			} else if(tokens[0] == "msg") {
				std::string output = "";
				for(int i=1; i<tokens.size(); i++) {
					output += tokens[i] + ' ';
				}
				output.pop_back(); // remove trailing ' '.
				console->add_line(sdl, font, output);
			} else if(tokens[0] == "tilechange") {
				if(tokens.size() >= 4) {
					int aspect, x, y;
					aspect = std::stoi(tokens[1]);
					x = std::stoi(tokens[2]);
					y = std::stoi(tokens[3]);
					grid->set(x, y, aspect);
				}
			} else if(tokens[0] == "EOF") {
				stop = true;
			} else {
				// unrecognized command.
			}
		} else {
			// empty line.
		}
	} // end of while.

	delete(socket);
	delete(window);
	if(grid) delete(grid);
	delete(tileset);
	delete(console);
	delete(textarea);
	delete(font);
	delete(sdl);
}

