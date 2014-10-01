#include "sdl.h"
#include "luaconfig.h"
#include "tileset.h"
#include "grid.h"
#include "window.h"
#include "socket.h"

#include <SDL2/SDL.h>
#include <thread>
#include <string>
#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

std::vector<std::string> split(std::string input, char delim) {
	std::vector<std::string> output;
	int begin = 0;
	int end = 0;
	while((end = input.find(delim, begin)) != std::string::npos) {
		output.push_back(input.substr(begin, end - begin));
		begin = end+1;
	}
	output.push_back(input.substr(begin)); // TODO
	return(output);
}

void sdl_loop(
	class Sdl * sdl,
	class Window * window,
	class Tileset * tileset,
	class Socket * socket);

void from_server_loop(
	class Socket * socket,
	class LuaConfig * conf);

class Grid * grid = NULL;

int main() {
	class LuaConfig conf = LuaConfig("conf.lua");
	class Sdl * sdl = new Sdl(
			conf.get_int("screen_width"),
			conf.get_int("screen_height"));
	class Tileset * tileset = new Tileset(sdl,
			std::string(conf.get_string("tileset_filename")),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));
	class Window * window = new Window(
			conf.get_int("screen_width") / conf.get_int("tileset_width"),
			conf.get_int("screen_height") / conf.get_int("tileset_height"),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));
	class Socket * socket = new Socket(
			conf.get_int("port"),
			conf.get_string("address"));

	try {
		sdl->set_icon(conf.get_string("icon"));
	} catch(...) { }
	try {
		sdl->set_title(conf.get_string("title"));
	} catch(...) { }

	std::thread * from_server_thread =
		new std::thread(from_server_loop, socket, &conf);
	std::thread * drawer =
		new std::thread(sdl_loop, sdl, window, tileset, socket);

	from_server_thread->join();
	delete(drawer);
	delete(socket);
	delete(window);
	if(grid) {
		delete(grid);
	}
	delete(tileset);
	delete(sdl);
}

void sdl_loop(
	class Sdl * sdl,
	class Window * window,
	class Tileset * tileset,
	class Socket * socket
) {
	while(true) {
		if(grid) {
			window->draw(sdl, grid, tileset);
		}
		sdl->next_frame();
		if(sdl->keydown(SDL_SCANCODE_UP)) {
			socket->send("move north\n");
		}
		if(sdl->keydown(SDL_SCANCODE_DOWN)) {
			socket->send("move south\n");
		}
		if(sdl->keydown(SDL_SCANCODE_LEFT)) {
			socket->send("move east\n");
		}
		if(sdl->keydown(SDL_SCANCODE_RIGHT)) {
			socket->send("move west\n");
		}
		if(sdl->keydown(SDL_SCANCODE_ESCAPE)) {
			socket->send("quit\n");
		}
// TODO : say.
	}
}

// TODO : URGENT : catch sigterm and sdl exit event.

void from_server_loop(
	class Socket * socket,
	class LuaConfig * conf
) {
	bool stop = false;
	while(not stop) {
		std::string input = socket->getline();
		std::vector<std::string> tokens = split(input, ' ');
		if(tokens.size() >= 1) {
			if(tokens[0] == "move") {
				// int player_id, x, y;
				// TODO : move or create player.
			} else if(tokens[0] == "exit") {
				// int player_id;
				// TODO : move or create player.
			} else if(tokens[0] == "floor") {
				if(tokens.size() >= 4) {
					int w, h;
					std::string name;
					w = std::stoi(tokens[1]);
					h = std::stoi(tokens[2]);
					name = tokens[3];
#ifdef DEBUG
					std::cout << "DEBUG: new floor ; w="<<w<<" ; h="<<h << std::endl;
#endif
					// TODO : lock
					if(grid != NULL) {
						delete(grid);
					}
					grid = new Grid(w, h,
							conf->get_int("tileset_width"),
							conf->get_int("tileset_height"));
					input = socket->getline();
					tokens = split(input, ',');
					if(tokens.size() == w*h) {
						for(int x=0; x<w; x++) {
							for(int y=0; y<h; y++) {
								grid->set(x, y, std::stoi(tokens.at(y*w+x)));
							}
						}
					}
#ifdef DEBUG
					else {
						std::cout << "DEBUG: provided "<<tokens.size()<<" tiles while expecting "<<w*h<<std::endl;
					}
#endif
					// TODO : unlock
				}
#ifdef DEBUG
				else {
					std::cout << "DEBUG: 'floor' arg's size is "<<tokens.size()<<", expecting 4 args." << std::endl;
				}
#endif
			} else if(tokens[0] == "EOF") {
				stop = true;
			} else {
				// unrecognized command.
			}
		} else {
			// empty line.
		}
	} // end of while.
}

