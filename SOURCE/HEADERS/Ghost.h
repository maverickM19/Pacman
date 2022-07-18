#pragma once
#include <SFML/Graphics.hpp>
#include "Animation.h"
#include "global.h"
#include <array>

class Ghost
{
public:
	Ghost(const sf::Vector2f &position, const Mode& mode, std::string file_name, std::string addon_file_name);
	void draw(sf::RenderWindow& window);
	void move(const size_t row, const float delta_time);
	void update_pathfinding(std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map, const sf::Vector2f targ_position);
	void revive();
	void switch_mode(const Mode& mode);
	void switch_texture(const Mode& mode);
	bool get_target_state() const;
	bool get_energizer_mode() const;
	short distance_to(const sf::Vector2f& position) const;
	Mode get_current_mode() const;
	sf::Vector2f get_position() const;
	sf::FloatRect get_hitbox() const;

	static size_t get_points_multiplier();
	static void set_points_multiplier(const size_t n);
private:
	sf::Vector2f position;
	sf::Vector2f restart_position;
	sf::Vector2f delta_position;
	sf::Vector2f current_target;
	sf::Vector2f target;
	std::vector<sf::Vector2f> target_position;

	Animation animation;
	sf::Sprite ghost_sprite;
	sf::Texture ghost_texture;
	sf::Texture scared_ghost_texture;
	Mode current_mode;
	float speed;
	float energizer_time;
	bool energizer;
	static size_t points_multiplier;
};

