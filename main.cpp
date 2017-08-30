#include "sdl.h"
#include "luaconfig.h"
#include "tileset.h"
#include "grid.h"
#include "window.h"
#include "socket.h"
#include "textrenderer.h"
#include "console.h"
#include "textarea.h"
#include "gauge.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Player {
	int id;
	int aspect;
	int x;
	int y;
	Player(int id, int aspect, int x, int y) :
		id(id), aspect(aspect), x(x), y(y) { }
};

struct Item {
	int id;
	int aspect;
	Item(int id, int aspect) :
		id(id), aspect(aspect) { }
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

void main_bis(std::string conf_filename);

int main(int argc, char * argv[]) {
	std::string conf_filename = argc > 1 ? argv[1] : "conf.lua";
	try {
		main_bis(conf_filename);
	} catch(std::string error) {
		std::cerr << "ERROR: " << error << std::endl;
	}
}

void main_bis(std::string conf_filename) {
	bool stop = false;
	int follow_id = 0;
	std::string input;
	std::vector<std::string> tokens;
	std::map<int, struct Player> players;
	std::map<std::string, class Gauge *> gauges;
	std::map<std::pair<int,int>,int> floorObjects;
	std::vector<struct Item> inventory;
	std::vector<struct Item> pickuplist;
	std::vector<std::string> macros = std::vector<std::string>(12, "");
	const SDL_Scancode key_fn[] = {
		SDL_SCANCODE_F1,
		SDL_SCANCODE_F2,
		SDL_SCANCODE_F3,
		SDL_SCANCODE_F4,
		SDL_SCANCODE_F5,
		SDL_SCANCODE_F6,
		SDL_SCANCODE_F7,
		SDL_SCANCODE_F8,
		SDL_SCANCODE_F9,
		SDL_SCANCODE_F10,
		SDL_SCANCODE_F11,
		SDL_SCANCODE_F12
	};
	const SDL_Scancode key_arrow[] = {
		SDL_SCANCODE_UP,
		SDL_SCANCODE_DOWN,
		SDL_SCANCODE_LEFT,
		SDL_SCANCODE_RIGHT
	};
	const std::string direction[] = {
		"north",
		"south",
		"west",
		"east"
	};
	class LuaConfig * conf = new LuaConfig(conf_filename);
	class Sdl * sdl = new Sdl(
			conf->get_int("screen_width"),
			conf->get_int("screen_height"));
	class Tileset * tileset = new Tileset(
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
	class Grid * inventory_gui = new Grid(
			conf->get_int("inventory_width"),
			(conf->get_int("screen_height")
				- conf->get_int("console_height")*font->lineskip()
				- conf->get_int("textarea_height")*font->lineskip())
					/ conf->get_int("tileset_height"),
			conf->get_int("tileset_width"),
			conf->get_int("tileset_height"));
	class Grid * pickuplist_gui = new Grid(
			conf->get_int("pickuplist_width"), // width
			(conf->get_int("screen_height")
				- conf->get_int("console_height")*font->lineskip()
				- conf->get_int("textarea_height")*font->lineskip())
					/ conf->get_int("tileset_height"), // height
			conf->get_int("tileset_width"), // tile_width
			conf->get_int("tileset_height"), // tile_height
			conf->get_int("screen_width") -
				(conf->get_int("pickuplist_width")
				* conf->get_int("tileset_width")), // x_shift
			0); // y_shift

	int movement_cooldown_max_frame = 10;
	int movement_cooldown_current = 0;

	class Socket * socket = NULL;
	class Grid * grid = NULL;
	int tileset_width = conf->get_int("tileset_width");
	int tileset_height = conf->get_int("tileset_height");
	int inventory_width = conf->get_int("inventory_width");
	int pickuplist_width = conf->get_int("pickuplist_width");

	for(std::string filename : conf->get_strings("tileset")) {
		tileset->append(sdl, filename);
	}

	if(conf->get_string("icon") != "") {
		sdl->set_icon(conf->get_string("icon"));
	}
	if(conf->get_string("title") != "") {
		sdl->set_title(conf->get_string("title"));
	}

	for(int i=0; i<12; i++) {
		std::string fn = "F"+std::to_string(i+1);
		if(conf->get_string(fn) != "") {
			macros[i] = conf->get_string(fn);
		}
	}

	socket = new Socket(
			conf->get_int("port"),
			conf->get_string("address"));
	socket->setBlocking(false);

	if(not socket->isOk()) {
		console->add_line(sdl, font,
			"Connexion ERROR ("
			+ conf->get_string("address")
			+ ":"
			+ std::to_string(conf->get_int("port"))
			+ ")");
		console->add_line(sdl, font, socket->getError());
		stop = true;
	}

	// delete(conf); // FIXME: It cause a double free error and I don't know why.

	while(not sdl->has_quit()) {

		// Draw.

		if(grid) {
			// Draw Floor.
			window->draw(sdl, grid, tileset);
			if(not sdl->key(SDL_SCANCODE_LCTRL)) {
				// Draw Objects.
				for(std::pair<std::pair<int,int>,int> it : floorObjects) {
					window->draw(sdl, it.second, tileset, it.first.first, it.first.second);
				}
				// Draw Players.
				for(std::pair<int, struct Player> it : players) {
					window->draw(sdl, it.second.aspect, tileset, it.second.x, it.second.y);
				}
			}
		}
		// Draw UI.
		console->draw(sdl);
		textarea->draw(sdl);
		int i = 0;
		for(std::pair<std::string, class Gauge *> it : gauges) {
			it.second->draw(sdl, inventory_width * tileset_width, i);
			i += it.second->getHeight();
		}
		i = 0;
		for(struct Item item : inventory) {
			inventory_gui->draw(sdl, tileset,
				i % inventory_width, i / inventory_width,
				item.aspect);
			i++;
		}
		i = 0;
		for(struct Item item : pickuplist) {
			pickuplist_gui->draw(sdl, tileset,
				i % pickuplist_width, i / pickuplist_width,
				item.aspect);
			i++;
		}

		sdl->next_frame();

		// Process Movement.
		if(movement_cooldown_current > 0) {
			movement_cooldown_current--;
		}
		if(movement_cooldown_current == 0
			and not sdl->key(SDL_SCANCODE_LSHIFT)
			and not sdl->key(SDL_SCANCODE_RSHIFT)
		) {
			for(int i=0; i<4; i++) {
				if(sdl->key(key_arrow[i])) {
					socket->send("move "+direction[i]+"\n");
					movement_cooldown_current = movement_cooldown_max_frame;
					break;
				}
			}
		}

		// Process Keyboard Input.

		textarea->add_string(sdl, font, sdl->get_text());
		if(sdl->key(SDL_SCANCODE_LSHIFT) || sdl->key(SDL_SCANCODE_RSHIFT)) {
			for(int i=0; i<4; i++) {
				if(sdl->keydown(key_arrow[i])) {
					textarea->add_string(sdl, font, direction[i]);
				}
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
		for(int i=0; i<12; i++) {
			if(sdl->keydown(key_fn[i])) {
				if(sdl->key(SDL_SCANCODE_LSHIFT)
						|| sdl->key(SDL_SCANCODE_RSHIFT)) {
					macros[i] = textarea->get_text();
				} else {
					textarea->add_string(sdl, font, macros[i]);
				}
			}
		}
		if(sdl->keydown(SDL_SCANCODE_ESCAPE)) {
			if(stop) {
				break;
			} else {
				if(sdl->key(SDL_SCANCODE_LSHIFT)
						|| sdl->key(SDL_SCANCODE_RSHIFT)) {
					socket->send("quit\n");
				} else {
					textarea->clear();
				}
			}
		}

		// Process Mouse Input.
		for(struct Clic clic : sdl->get_clics()) {
			int objx, objy;
			std::string command;
			int gui_width;
			class Grid * gui = NULL;
			std::vector<struct Item> * itemlist = NULL;
			struct Item * item = NULL;
			if(inventory_gui->is_clic_in(clic.x, clic.y)) {
				gui = inventory_gui;
				gui_width = inventory_width;
				itemlist = &inventory;
				command = "drop";
			} else if(pickuplist_gui->is_clic_in(clic.x, clic.y)) {
				gui = pickuplist_gui;
				gui_width = pickuplist_width;
				itemlist = &pickuplist;
				command = "pickup";
			}
			if(gui) {
				objx = gui->get_clic_x(clic.x);
				objy = gui->get_clic_y(clic.y);
				try {
					item = &(itemlist->at(objy * gui_width + objx));
				} catch(...) { }
			}
			if(item) {
				if(clic.button == SDL_BUTTON_LEFT) {
					// Add object's id to console.
					textarea->add_string(sdl, font, std::to_string(item->id));
				} else if(clic.button == SDL_BUTTON_RIGHT) {
					// send pickup/drop object's id.
					socket->send(command+" "+std::to_string(item->id)+"\n");
				}
			}
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
						pickuplist.clear();
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
					floorObjects.clear();

					if(grid) delete(grid);
					grid = new Grid(w, h, tileset_width, tileset_height);

					socket->setBlocking(true);
					input = socket->getline();
					socket->setBlocking(false);
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
			} else if(tokens[0] == "floorchange") {
				if(tokens.size() >= 4) {
					int aspect, x, y;
					aspect = std::stoi(tokens[1]);
					x = std::stoi(tokens[2]);
					y = std::stoi(tokens[3]);
					grid->set(x, y, aspect);
				}
			} else if(tokens[0] == "gauge") {
				if(tokens.size() >= 6) {
					std::string name = tokens[1];
					int val = std::stoi(tokens[2]);
					int max = std::stoi(tokens[3]);
					int full = std::stoi(tokens[4]);
					int empty = std::stoi(tokens[5]);
					try {
						// Gauge already exists: update.
						gauges.at(name)->set(val, max);
						gauges.at(name)->setAspect(
								tileset->get(full), tileset->get(empty));
					} catch(...) {
						// Gauge doesn't exist: add new.
						class Gauge * gauge = new Gauge(
								sdl, font, name,
								tileset->get(full), tileset->get(empty));
						gauge->set(val, max);
						gauges.insert(std::pair<std::string, class Gauge *>(
								name, gauge));
					}
				}
			} else if(tokens[0] == "nogauge") {
				if(tokens.size() >= 2) {
					std::string name = tokens[1];
					try {
						delete(gauges.at(name));
						gauges.erase(name);
					} catch(...) { }
				}
			} else if(tokens[0] == "obj") {
				if(tokens.size() >= 4) {
					int aspect = std::stoi(tokens[1]);
					int x = std::stoi(tokens[2]);
					int y = std::stoi(tokens[3]);
					floorObjects[std::pair<int,int>(x,y)] = aspect;
				}
			} else if(tokens[0] == "noobj") {
				if(tokens.size() >= 3) {
					int x = std::stoi(tokens[1]);
					int y = std::stoi(tokens[2]);
					floorObjects.erase(std::pair<int,int>(x,y));
				}
			} else if(tokens[0] == "invent") {
				if(tokens.size() >= 3) {
					int id = std::stoi(tokens[1]);
					int aspect = std::stoi(tokens[2]);
					inventory.push_back(Item(id,aspect));
				}
			} else if(tokens[0] == "noinvent") {
				if(tokens.size() >= 2) {
					int id = std::stoi(tokens[1]);
					for(auto it = inventory.begin(); it != inventory.end(); it++) {
						if(it->id == id) {
							inventory.erase(it);
							break;
						}
					}
				}
			} else if(tokens[0] == "addpickuplist") {
				if(tokens.size() >= 3) {
					int id = std::stoi(tokens[1]);
					int aspect = std::stoi(tokens[2]);
					pickuplist.push_back(Item(id,aspect));
				}
			} else if(tokens[0] == "rempickuplist") {
				if(tokens.size() >= 2) {
					int id = std::stoi(tokens[1]);
					for(auto it = pickuplist.begin(); it != inventory.end(); it++) {
						if(it->id == id) {
							pickuplist.erase(it);
							break;
						}
					}
				}
			} else if(tokens[0] == "EOF") {
				console->add_line(sdl, font, "DISCONNECTED");
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

