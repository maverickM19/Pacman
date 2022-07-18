#include <array>
#include <string>
#include "Ghost.h"
#include "global.h"

Ghost::Ghost(const sf::Vector2f& position, const Mode& mode, std::string main_file_name, std::string addon_file_name)
{
	restart_position = position;
	this->position = position;

	ghost_texture.loadFromFile(main_file_name);
	scared_ghost_texture.loadFromFile(addon_file_name);
	animation = Animation(&ghost_texture, sf::Vector2u(2, 4), 0.1);
	ghost_sprite.setTexture(ghost_texture);
	ghost_sprite.setTextureRect(animation.rectangle);
	ghost_sprite.setScale(1.9, 1.9);

	speed = 2;
	current_mode = mode;
	energizer_time = 0;
	energizer = false;
}

size_t Ghost::points_multiplier = 1;

sf::FloatRect Ghost::get_hitbox() const
{
	return sf::FloatRect(position.x + 5, position.y + 5, animation.rectangle.width * 1.9, animation.rectangle.height * 1.9);
}

size_t Ghost::get_points_multiplier() 
{
	return points_multiplier;
}

void Ghost::set_points_multiplier(const size_t n) 
{
	points_multiplier = n;
}

bool Ghost::get_target_state()
{
	return (target_position.empty());
}

sf::Vector2f Ghost::get_position() const
{
	return position;
}

bool Ghost::get_energizer_mode() const 
{
	return energizer;
}

void Ghost::switch_mode(const Mode& mode)
{
	current_mode = mode;
}

Mode Ghost::get_current_mode() const
{
	return current_mode;
}

void Ghost::revive() 
{
	position = restart_position;
	target_position.clear();
	switch_texture(Mode::SCATTER);
}

void Ghost::switch_texture(const Mode& mode)
{
	if (mode != Mode::SCATTER_ENERGIZER)
	{
		ghost_sprite.setTexture(ghost_texture);
		animation = Animation(&ghost_texture, sf::Vector2u(2, 4), 0.1);
		speed = 2;
		current_mode = Mode::SCATTER;
		energizer = false;
	}
	else
	{
		ghost_sprite.setTexture(scared_ghost_texture);
		animation = Animation(&scared_ghost_texture, sf::Vector2u(2, 4), 0.1);
		speed = 1;
		current_mode = Mode::SCATTER;
		energizer = true;
		energizer_time = 0;
		points_multiplier = 1;
	}

	ghost_sprite.setTextureRect(animation.rectangle);
}

void Ghost::draw(sf::RenderWindow& window) 
{
	ghost_sprite.setPosition(position.x - 5, position.y + CELL_SIZE - 5);
	ghost_sprite.setScale(1.9, 1.9);
	window.draw(ghost_sprite);
}

short Ghost::distance_to(const sf::Vector2f& position) 
{
	return sqrt((this->position.x - position.x) * (this->position.x - position.x) + (this->position.y - position.y) * (this->position.y - position.y));
}

void Ghost::move(const size_t row, const float delta_time) 
{
	if (target_position.empty())
	{
		return;
	}

	if ((fabs(current_target.x - position.x) < speed) && (fabs(current_target.y - position.y) < speed))
	{
		target_position.pop_back();
		position = current_target;

		if (target_position.empty())
		{
			return;
		}

		current_target = target_position[target_position.size() - 1];
	}


	if (energizer)
	{
		energizer_time += delta_time;
	}

	if (position.y < current_target.y)
	{
		position.y += speed;
		if (!energizer)
		{
			animation.update(1, delta_time);
		}
		else
		{
			if (energizer_time > ENERGIZER_TIME - 3)
			{
				animation.update(1, delta_time);
			}
			else
			{
				animation.update(0, delta_time);
			}
		}
	}
	else if (position.y > current_target.y)
	{
		position.y -= speed;
		if (!energizer)
		{
			animation.update(0, delta_time);
		}
		else
		{
			if (energizer_time > ENERGIZER_TIME - 3)
			{
				animation.update(1, delta_time);
			}
			else
			{
				animation.update(0, delta_time);
			}
		}
	}

	if (position.x < current_target.x)
	{
		position.x += speed;
		if (!energizer)
		{
			animation.update(3, delta_time);
		}
		else
		{
			if (energizer_time > ENERGIZER_TIME - 3)
			{
				animation.update(1, delta_time);
			}
			else
			{
				animation.update(0, delta_time);
			}
		}
	}
	else if (position.x > current_target.x)
	{
		position.x -= speed;
		if (!energizer)
		{
			animation.update(2, delta_time);
		}
		else
		{
			if (energizer_time > ENERGIZER_TIME - 3)
			{
				animation.update(1, delta_time);
			}
			else
			{
				animation.update(0, delta_time);
			}
		}
	}

	ghost_sprite.setTextureRect(animation.rectangle);
}

void Ghost::update_pathfinding(std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map, const sf::Vector2f targ_position) 
{
	std::vector<Cell*> open_list;
	std::vector<Cell*> closed_list;
	std::vector<Cell*> path_list;
	std::vector<Cell*>::iterator itr;
	Cell* current_node = nullptr;

	Cell* start_node = &map[std::round(position.y / CELL_SIZE)][std::round(position.x / CELL_SIZE)];
	Cell* end_node = &map[std::round(targ_position.y / CELL_SIZE)][std::round(targ_position.x / CELL_SIZE)];

	if (start_node == end_node)
	{
		target_position.clear();

		return;
	}

	for (short i = 0; i < MAP_HEIGHT; i++)			//calculting H value for each node 
	{
		for (short j = 0; j < MAP_WIDTH; j++)
		{
			map[i][j].H = (fabs(end_node->row_index - i) + fabs(end_node->column_index - j)) * 10;
		}
	}

	open_list.push_back(start_node);

	int lowest_F, lowest_H;
	std::vector<Cell*> neighbour_nodes;
	Cell* node = nullptr;
	while (!open_list.empty())
	{
		lowest_F = INT_MAX;
		lowest_H = INT_MAX;
		for (auto node : open_list)
		{
			if (node->F < lowest_F)
			{
				lowest_F = node->F;
				lowest_H = node->H;
				current_node = node;
			}
			else if (node->F == lowest_F)
			{
				if (node->H < lowest_H)
				{
					lowest_F = node->F;
					lowest_H = node->H;
					current_node = node;
				}
			}
		}

		itr = std::find(open_list.begin(), open_list.end(), current_node);
		if (itr != open_list.end())
		{
			open_list.erase(itr);
		}

		closed_list.push_back(current_node);

		for (short i = -1; i < 2; i++)		//adding all nodes to the list that are neighbour to our current node
		{
			for (short j = -1; j < 2; j++)		//disable diagonal movement
			{
				if (i != 0 || j != 0)
				{
					if (j + i != 0 && fabs(j + i) != 2)
					{
						if ((current_node->row_index + i >= 0 && current_node->row_index + i < MAP_HEIGHT)			//check it's boundaries
							&& (current_node->column_index + j >= 0 && current_node->column_index + j < MAP_WIDTH))
						{
							itr = std::find(closed_list.begin(), closed_list.end(), &map[current_node->row_index + i][current_node->column_index + j]);
							if (map[current_node->row_index + i][current_node->column_index + j].type != Cell_Type::WALL
								&& itr == closed_list.end())
							{
								neighbour_nodes.push_back(&map[current_node->row_index + i][current_node->column_index + j]);

								node = &map[current_node->row_index + i][current_node->column_index + j];

								if (node == end_node)
								{
									node->parent = current_node;

									while (node != nullptr)
									{
										path_list.push_back(node);
										node = node->parent;
									}

									open_list.clear();
									i = 2;
									break;
								}

								itr = std::find(open_list.begin(), open_list.end(), &map[current_node->row_index + i][current_node->column_index + j]);
								if (itr == open_list.end())
								{
									open_list.push_back(node);

									node->parent = current_node;

									int value = (j + i == 0 || fabs(j + i) == 2) ? 14 : 10;
									node->G = current_node->G + value;
									node->F = node->G + node->H;
								}
								else
								{
									int value = (j + i == 0 || fabs(j + i) == 2) ? 14 : 10;
									int G = current_node->G + value;
									if (G < node->G)
									{
										node->parent = current_node;
										node->G = G;
									}
									node->F = node->G + node->H;
								}

							}
						}
					}
				}
			}
		}
	}
		target_position.clear();
		for (auto node : path_list)
		{
			target_position.push_back(sf::Vector2f(node->column_index * CELL_SIZE + CELL_SIZE / 6, node->row_index * CELL_SIZE + CELL_SIZE / 6));
		}

		current_target = target_position[target_position.size() - 2];
}
