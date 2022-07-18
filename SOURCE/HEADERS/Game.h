#pragma once
#include "Pacman.h"
#include "Ghost.h"

class Game
{
public:
	Game();

	void run();
private:
	void get_map();
	void draw_map();
	void draw_ghosts();
	void move_ghosts(const float delta_time);
	void reset_cells();
	void level_up();
	void set_path(const Key& key, size_t& corner, float& second);
private:
	std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> map;
	std::vector<std::unique_ptr<Ghost>> ghosts;
	sf::Vector2f corners[4];
	Pacman pacman;
	Mode current_mode;
	sf::Font font;

	sf::Texture map_texture;
	sf::Texture items_texture;
	sf::RenderWindow window;

	bool game_over;
	bool play_pacman_death_anim;
	float scatter_time;
	float chase_time;

	unsigned int highscore;
	size_t level;
};

