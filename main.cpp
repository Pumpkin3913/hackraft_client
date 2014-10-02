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
#include <map>

#ifdef DEBUG
#include <iostream>
#endif

struct Player {
	int id;
	int aspect;
	int x;
	int y;
	Player(int id, int aspect, int x, int y) :
		id(id), aspect(aspect), x(x), y(y) { }
};

void sdl_loop(
	class Sdl * sdl,
	class Tileset * tileset,
	class Socket * socket);

void from_server_loop(
	class Socket * socket,
	class LuaConfig * conf);

class Grid * g_grid = NULL;
std::map<int, struct Player> * g_players = NULL;
class Window * g_window = NULL;

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
	class LuaConfig conf = LuaConfig("conf.lua");
	class Sdl * sdl = new Sdl(
			conf.get_int("screen_width"),
			conf.get_int("screen_height"));
	class Tileset * tileset = new Tileset(sdl,
			std::string(conf.get_string("tileset_filename")),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));
	class Socket * socket = new Socket(
			conf.get_int("port"),
			conf.get_string("address"));
	g_window = new Window(
			conf.get_int("screen_width") / conf.get_int("tileset_width"),
			conf.get_int("screen_height") / conf.get_int("tileset_height"),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));

	try {
		sdl->set_icon(conf.get_string("icon"));
	} catch(...) { }
	try {
		sdl->set_title(conf.get_string("title"));
	} catch(...) { }

	std::thread * from_server_thread =
		new std::thread(from_server_loop, socket, &conf);
	std::thread * drawer =
		new std::thread(sdl_loop, sdl, tileset, socket);

	from_server_thread->join();
	delete(drawer);
	delete(socket);
	delete(g_window);
	if(g_grid) delete(g_grid);
	if(g_players) delete(g_players);
	delete(tileset);
	delete(sdl);
}

void sdl_loop(
	class Sdl * sdl,
	class Tileset * tileset,
	class Socket * socket
) {
	while(true) {
		if(g_grid) {
			g_window->draw(sdl, g_grid, tileset);
			if(g_players && not sdl->key(SDL_SCANCODE_LSHIFT)) {
				for(std::pair<int, struct Player> it : *g_players) {
					g_window->draw(sdl, it.second.aspect, tileset, it.second.x, it.second.y);
				}
			}
		}
		sdl->next_frame();
		if(sdl->keydown(SDL_SCANCODE_UP)) {
			socket->send("move north\n");
		}
		if(sdl->keydown(SDL_SCANCODE_DOWN)) {
			socket->send("move south\n");
		}
		if(sdl->keydown(SDL_SCANCODE_LEFT)) {
			socket->send("move west\n");
		}
		if(sdl->keydown(SDL_SCANCODE_RIGHT)) {
			socket->send("move east\n");
		}
		if(sdl->keydown(SDL_SCANCODE_ESCAPE)) {
			socket->send("quit\n");
		}
// TODO : say.
	}
}

// TODO : catch sigterm and sdl exit event.

void from_server_loop(
	class Socket * socket,
	class LuaConfig * conf
) {
	bool stop = false;
	int follow_id = 0;
	while(not stop) {
		std::string input = socket->getline();
		std::vector<std::string> tokens = split(input, ' ');
		if(tokens.size() >= 1) {
			if(tokens[0] == "move") {
				if(tokens.size() >= 5 && g_players) {
					int id = std::stoi(tokens[1]);
					int aspect = std::stoi(tokens[2]);
					int x = std::stoi(tokens[3]);
					int y = std::stoi(tokens[4]);
					try {
						g_players->at(id).aspect = aspect;
						g_players->at(id).x = x;
						g_players->at(id).y = y;
					} catch(...) {
						g_players->insert(std::pair<int, struct Player>(id, Player(id, aspect, x, y)));
					}
					if(id == follow_id) {
						g_window->set_center(x, y);
					}
				}
			} else if(tokens[0] == "exit") {
				if(tokens.size() >= 2 && g_players) {
					int id = std::stoi(tokens[1]);
					g_players->erase(id);
				}
			} else if(tokens[0] == "follow") {
				if(tokens.size() >= 2) {
					follow_id = std::stoi(tokens[1]);
					try {
						int x = g_players->at(follow_id).x;
						int y = g_players->at(follow_id).y;
						g_window->set_center(x, y);
					} catch(...) { }
				}
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
					if(g_players) delete(g_players);
					g_players = new std::map<int, struct Player>();

					// TODO : lock
					if(g_grid) delete(g_grid);
					g_grid = new Grid(w, h,
							conf->get_int("tileset_width"),
							conf->get_int("tileset_height"));

					input = socket->getline();
					tokens = split(input, ',');
					if(tokens.size() == w*h) {
						for(int x=0; x<w; x++) {
							for(int y=0; y<h; y++) {
								g_grid->set(x, y, std::stoi(tokens.at(y*w+x)));
							}
						}
					}
					// TODO : unlock
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
}

