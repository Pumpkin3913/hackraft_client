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
#include <thread>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <mutex>

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
	class Socket * socket,
	class TextArea * textarea,
	class Console * console,
	class TextRenderer * font);

void from_server_loop(
	class Sdl * sdl,
	class Socket * socket);

class Grid * g_grid = NULL;
std::map<int, struct Player> * g_players = NULL;
class Window * g_window = NULL;
std::mutex g_mutex;
int g_tileset_width;
int g_tileset_height;
std::list<std::string> g_to_console;

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
	class LuaConfig * conf = new LuaConfig("conf.lua");
	class Sdl * sdl = new Sdl(
			conf->get_int("screen_width"),
			conf->get_int("screen_height"));
	class Tileset * tileset = new Tileset(sdl,
			std::string(conf->get_string("tileset_filename")),
			conf->get_int("tileset_width"),
			conf->get_int("tileset_height"));
	class Socket * socket = new Socket(
			conf->get_int("port"),
			conf->get_string("address"));
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
	g_window = new Window(
			conf->get_int("screen_width") / conf->get_int("tileset_width"),
			(conf->get_int("screen_height")
				- conf->get_int("console_height")*font->lineskip()
				- conf->get_int("textarea_height")*font->lineskip())
					/ conf->get_int("tileset_height"),
			conf->get_int("tileset_width"),
			conf->get_int("tileset_height"));

	g_tileset_width = conf->get_int("tileset_width");
	g_tileset_height = conf->get_int("tileset_height");

	if(conf->get_string("icon") != "") {
		sdl->set_icon(conf->get_string("icon"));
	}
	if(conf->get_string("title") != "") {
		sdl->set_title(conf->get_string("title"));
	}

	// delete(conf); // FIXME: It cause a double free error and I don't know why.

	std::thread * from_server_thread =
		new std::thread(from_server_loop, sdl, socket);
	std::thread * drawer =
		new std::thread(sdl_loop, sdl, tileset, socket, textarea, console, font);

	from_server_thread->join();
	drawer->detach();
	delete(drawer);
	delete(socket);
	delete(g_window);
	if(g_grid) delete(g_grid);
	if(g_players) delete(g_players);
	delete(tileset);
	delete(console);
	delete(textarea);
	delete(font);
	delete(sdl);
}

void sdl_loop(
	class Sdl * sdl,
	class Tileset * tileset,
	class Socket * socket,
	class TextArea * textarea,
	class Console * console,
	class TextRenderer * font
) {
	g_mutex.lock();
	while(true) {
		while(not g_to_console.empty()) {
			console->add_line(sdl, font, g_to_console.back());
			g_to_console.pop_back();
		}
		console->draw(sdl);
		textarea->draw(sdl);
		if(g_grid) {
			g_window->draw(sdl, g_grid, tileset);
			if(g_players && not sdl->key(SDL_SCANCODE_LCTRL)) {
				for(std::pair<int, struct Player> it : *g_players) {
					g_window->draw(sdl, it.second.aspect, tileset, it.second.x, it.second.y);
				}
			}
		}
		g_mutex.unlock();
		sdl->next_frame();
		g_mutex.lock();
		for(char c : sdl->get_text()) {
			textarea->add_char(sdl, font, c);
		}
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
	}
}

void from_server_loop(
	class Sdl * sdl,
	class Socket * socket
) {
	bool stop = false;
	int follow_id = 0;
	g_mutex.lock();
	while(not stop) {
		g_mutex.unlock();
		std::string input = socket->getline();
		std::vector<std::string> tokens = split(input, ' ');
		g_mutex.lock();
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
					if(g_players) delete(g_players);
					g_players = new std::map<int, struct Player>();

					if(g_grid) delete(g_grid);
					g_grid = new Grid(w, h, g_tileset_width, g_tileset_height);

					input = socket->getline();
					tokens = split(input, ',');
					if(tokens.size() == w*h) {
						for(int x=0; x<w; x++) {
							for(int y=0; y<h; y++) {
								g_grid->set(x, y, std::stoi(tokens.at(y*w+x)));
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
				g_to_console.push_front(output);
			} else if(tokens[0] == "tilechange") {
				if(tokens.size() >= 4) {
					int aspect, x, y;
					aspect = std::stoi(tokens[1]);
					x = std::stoi(tokens[2]);
					y = std::stoi(tokens[3]);
					g_grid->set(x, y, aspect);
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

