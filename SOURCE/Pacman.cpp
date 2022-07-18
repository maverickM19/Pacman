#include "Pacman.h"
#include <array>

Pacman::Pacman()
{
	player_texture.loadFromFile("pacman.png");
	animation = Animation(&player_texture, sf::Vector2u(6, 4), ANIMATION_SPEED);

	player_texture_death.loadFromFile("pacman_death5.png");
	animation_death = Animation(&player_texture_death, sf::Vector2u(12, 1), 0.2);

	player_sprite.setTexture(player_texture);
	player_sprite.setTextureRect(animation.rectangle);

	speed = 2;
	lives = 3;
	points = 0;
	energizer = false;
	dead = false;
}

sf::FloatRect Pacman::get_hitbox() const
{
	return sf::FloatRect(position.x - 3, position.y - 1, animation.rectangle.width * 1.2, animation.rectangle.height * 1.2);
}

unsigned int Pacman::get_points() const 
{
	return points;
}

size_t Pacman::get_lives() const 
{
	return lives;
}

void Pacman::set_restart_position(sf::Vector2f position) 
{
	this->restart_position = position;
}

void Pacman::set_position(sf::Vector2f position) 
{
	this->position = position;
}

sf::Vector2f Pacman::get_position() const 
{
	return position;
}

void Pacman::set_energizer(const bool energizer)
{
	this->energizer = energizer;
}

void Pacman::restart()
{
	if (!lives)
	{
		points = 0;
		lives = 3;
	}

	position = restart_position;
}

void Pacman::draw(sf::RenderWindow& window)
{
	player_sprite.setPosition(position.x + 3, position.y + CELL_SIZE + 1);
	player_sprite.setScale(1.9f, 1.9f);

	window.draw(player_sprite);
}

bool Pacman::check_collision(const std::unique_ptr<Ghost>& ghost)
{
	if (!((fabs(position.x - ghost->get_position().x) < CELL_SIZE / 3) && (fabs(position.y - ghost->get_position().y) < CELL_SIZE / 3)))
	{
		return false;
	}

	if (!ghost->get_energizer_mode())
	{
		--lives;
		return true;
	}
	
	points += Ghost::get_points_multiplier() * 200;
	Ghost::set_points_multiplier(Ghost::get_points_multiplier() * 2);
	ghost->revive();
	return false;
}

bool Pacman::play_death_animation(const float delta_time, sf::RenderWindow& window)
{
	sf::Sprite sprite;
	animation_death.update(0, delta_time);
	sprite.setTexture(player_texture_death);
	sprite.setTextureRect(animation_death.rectangle);
	sprite.setPosition(position.x, position.y + CELL_SIZE);
	sprite.setScale(2.0f, 2.0f);

	window.draw(sprite);

	if (animation_death.get_frame() == 11)
	{
		animation_death.set_frame(0);
		return true;
	}

	return false;
}

bool Pacman::move(Key& key, Key& next_key, const float delta_time, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map)
{
	if (key == Key::KEY_STOP)
	{
		return false;
	}

	if (next_key != Key::KEY_STOP)
	{
		switch (next_key)
		{
		case Key::KEY_LEFT:
			delta_position.x = -speed;
			delta_position.y = 0;
			break;
		case Key::KEY_RIGHT:
			delta_position.x = speed;
			delta_position.y = 0;
			break;
		case Key::KEY_UP:
			delta_position.x = 0;
			delta_position.y = -speed;
			break;
		case Key::KEY_DOWN:
			delta_position.x = 0;
			delta_position.y = speed;
			break;
		default:
			break;
		}

		if (check_map_collision(next_key, sf::Vector2f(position + delta_position), map))
		{
			position.x += delta_position.x;
			position.y += delta_position.y;
			if (map[round(position.y / CELL_SIZE)][round(position.x / CELL_SIZE)].has_item)
			{
				points++;
				map[round(position.y / CELL_SIZE)][round(position.x / CELL_SIZE)].has_item = false;
			}
	
			animation.update(static_cast<int>(next_key), delta_time);

			player_sprite.setTextureRect(animation.rectangle);

			key = next_key;

			next_key = Key::KEY_STOP;

			return false;
		}
	}

	switch (key)
	{
	case Key::KEY_LEFT:
		delta_position.x = -speed;
		delta_position.y = 0;
		break;
	case Key::KEY_RIGHT:
		delta_position.x = speed;
		delta_position.y = 0;
		break;
	case Key::KEY_UP:
		delta_position.x = 0;
		delta_position.y = -speed;
		break;
	case Key::KEY_DOWN:
		delta_position.x = 0;
		delta_position.y = speed;
		break;
	default:
		break;
	}

	if (!check_map_collision(key, sf::Vector2f(position + delta_position), map))
	{
		return false;
	}

	position.x += delta_position.x;
	position.y += delta_position.y;

	animation.update(static_cast<int>(key), delta_time);

	player_sprite.setTextureRect(animation.rectangle);

	sf::Vector2f map_position;
	map_position.x = round(position.x / CELL_SIZE);
	map_position.y = round(position.y / CELL_SIZE);

	if (!((map_position.x >= 0 && map_position.x < MAP_WIDTH - 1) && (map_position.y >= 0 && map_position.y < MAP_HEIGHT - 1)))
	{
		return false;
	}

	if (map[map_position.y][map_position.x].has_item)
	{
		points++;
		map[map_position.y][map_position.x].has_item = false;
		if (map[map_position.y][map_position.x].item == Item_Type::ENERGIZER)
		{
			return true;
		}
	}

	return false;
}

bool Pacman::check_map_collision(Key& key, const sf::Vector2f& curr_position, const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map)
{
	sf::Vector2f map_position;

	for (size_t i = 0; i < 4; i++)
	{
		switch (i)
		{
			case 0:
			{
				map_position.x = floor(((curr_position.x) / CELL_SIZE));
				map_position.y = floor((curr_position.y / CELL_SIZE));

				break;
			}
			case 1:
			{
				map_position.x = ceil(((curr_position.x - speed + 1) / CELL_SIZE));
				map_position.y = floor(((curr_position.y) / CELL_SIZE));

				break;
			}
			case 2:
			{
				map_position.x = floor((curr_position.x / CELL_SIZE));
				map_position.y = ceil(((curr_position.y - speed + 1) / CELL_SIZE));

				break;
			}
			case 3:
			{
				map_position.x = ceil((curr_position.x - speed + 1) / CELL_SIZE);
				map_position.y = ceil((curr_position.y - speed + 1) / CELL_SIZE);
				break;
			}
		}

		if (!((map_position.x >= 0 && map_position.x <= MAP_WIDTH - 1)))
		{
			if (position.x <= 0 || position.x >= ((MAP_WIDTH - 1) * CELL_SIZE))
			{
				position.x = (key == Key::KEY_LEFT) ? ((MAP_WIDTH - 1) * CELL_SIZE) : 0;
			}

			return true;
		}

		if ((map[map_position.y][map_position.x].type != Cell_Type::FLOOR))
		{
			return false;
		}
	}

	return true;
}
