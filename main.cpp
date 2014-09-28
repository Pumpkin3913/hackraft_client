#include "sdl.h"
#include "luaconfig.h"
#include "tileset.h"
#include "grid.h"
#include "window.h"

#include <SDL2/SDL.h>
#include <thread>

int main() {
	class LuaConfig conf = LuaConfig("conf.lua");
	class Sdl sdl(
			conf.get_int("screen_width"),
			conf.get_int("screen_height"));
	class Tileset * tileset = new Tileset(sdl,
			conf.get_string("tileset_filename"),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));
	class Grid * grid = NULL;
	class Window * window = new Window(
			conf.get_int("screen_width") / conf.get_int("tileset_width"),
			conf.get_int("screen_height") / conf.get_int("tileset_height"),
			conf.get_int("tileset_width"),
			conf.get_int("tileset_height"));
	class Socket * socket = new Socket(
			conf.get_int("port"),
			conf.get_string("address"));

	sdl->set_icon(conf.get_string("icon"));
	sdl->set_title(conf.get_string("title"));

	std::thread * from_server_thread = new std::thread(from_server_loop, socket, &grid, conf);
	std::thread * drawer = new std::thread(sdl_loop, sdl, window, &grid, tileset);

	from_server_thread->join();
	delete(sdl_thread);

	delete(socket);
}

void sdl_loop(class Sdl * sdl, class Window * window, class Grid ** grid, class Tileset * tileset) {
	while(true) {
		if(*grid) {
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
			socket->send("exit\n"); // XXX Rather "quit", isn't it?
		}
// TODO : say.
	}
}

void from_server_loop(class Socket * socket, class Grid ** grid, class LuaConfig * conf) {
	bool stop = false;
	while(not stop) {
		std::string input;
		// try { // XXX
			input = socket->receive();
		/* XXX //
		} catch {
			break;
		}
		// XXX */
		std::size_t separator = input.find_first_of(' ');
		if(separator != std::string::npos) {
			std::string cmd = input.substr(0, separator);
			std::string arg = input.substr(separator+1);
			if(cmd == "move") {
				int player_id, x, y;
				sscanf(arg.c_str(), "%d %d %d", &player_id, &x, &y);
				// TODO : move or create player.
			} else if(cmd == "exit") {
				int player_id = std::stoi(arg);
				// TODO : remove player.
			} else if(cmd == "floor") {
				separator = arg.find_first_of(' ');
				if(separator != std::string::npos) {
					int w, h;
					std::string name = arg.substr(0, separator);
					sscanf(arg.substr(separator+1).c_str(), "%d %d", &w, &h);
					std::string tiles = socket->receive();
					// TODO : lock
					if(*grid != NULL) {
						delete(*grid);
					}
					*grid = new Grid(w, h,
							conf.get_int("tileset_width"),
							conf.get_int("tileset_height"));
					std::vector<int> tiles_list;
					std::size_t begin = 0;
					std::size_t end = tiles.find_first_of(',');
					while(end != std::string::npos) {
						tiles_list.push_back(std::stoi(tiles.substr(begin, end)));
						begin = end;
						end = tile.find_first_of(',', begin);
					}
					tiles_list.push_back(std::stoi(tiles.substr(begin))); // last.
					for(int x=0; x<w; x++) {
						for(int y=0; y<h; y++) {
							*grid->set(x, y, tiles_list.at(y*w+x));
						}
					}
					// TODO : unlock
				}
			} else if(cmd == "EOF") {
				stop = true;
			} else {
			}
		}
	}
}

