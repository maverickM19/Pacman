#pragma once
#include "Animation.h" 
#include "Ghost.h"

class Pacman
{
public:
	Pacman();
	void set_restart_position(sf::Vector2f position);
	void set_position(sf::Vector2f position);
	void draw(sf::RenderWindow& window);
	void set_energizer(const bool energizer);
	void restart();
	bool move(Key &key, Key& next_key, const float delta_time, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map);
	bool check_map_collision(Key& key, const sf::Vector2f& curr_position, const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map);
	bool check_collision(const std::unique_ptr<Ghost>& ghost);
	bool play_death_animation(const float delta_time, sf::RenderWindow& window);
	unsigned int get_points() const;
	sf::Vector2f get_position() const;
	sf::FloatRect get_hitbox() const;
	size_t get_lives() const;
private:
	sf::Vector2f position;
	sf::Vector2f restart_position;
	sf::Vector2f delta_position;

	sf::Sprite player_sprite;
	sf::Texture player_texture;
	sf::Texture player_texture_death;

	Animation animation;
	Animation animation_death;

	float speed;
	size_t lives;
	unsigned int points;
	bool energizer;
	bool dead;
};

