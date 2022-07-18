#include "Game.h"
#include <array>
#include <chrono>

Game::Game() : window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pac-man Remake")
{
	window.setPosition(sf::Vector2i(500, 0));
	map_texture.loadFromFile(MAP_TEXTURE_PATH);
	items_texture.loadFromFile("items.png");
	current_mode = Mode::CHASE;
	corners[0] = sf::Vector2f(1 * CELL_SIZE, 1 * CELL_SIZE);
	corners[1] = sf::Vector2f(1 * CELL_SIZE, (MAP_HEIGHT - 2) * CELL_SIZE);
	corners[2] = sf::Vector2f((MAP_WIDTH - 2) * CELL_SIZE, 1 * CELL_SIZE);
	corners[3] = sf::Vector2f((MAP_WIDTH - 2) * CELL_SIZE, (MAP_HEIGHT - 2) * CELL_SIZE);
	font.loadFromFile("arcade_font.ttf");

	game_over = false;
	play_pacman_death_anim = false;
	scatter_time = SCATTER_TIME;
	chase_time = CHASE_TIME;

	level = 1;
	highscore = 0;
}

void Game::run() 
{
	std::chrono::microseconds lag(0);
	std::chrono::steady_clock::time_point previous_time = std::chrono::steady_clock::now();
	Key key = Key::KEY_STOP;
	Key next_key = Key::KEY_STOP;

	size_t corner = 4;
	float dt = 0, scatter = 0, timer = 0, energizer_timer = 0, second = 0;
	unsigned int current_points;
	sf::Clock clock, mode_clock, energizer_clock;

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(13);
	text.setStyle(sf::Text::Bold);
	text.setFillColor(sf::Color::White);

	get_map();

	while (window.isOpen())
	{
		std::chrono::microseconds delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time);
		lag += delta_time;
		previous_time += delta_time;
		while (std::chrono::microseconds(FRAME_DURATION) < lag)
		{
			lag -= std::chrono::microseconds(FRAME_DURATION);

			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window.close();
				}

				if (event.type == sf::Event::KeyPressed)
				{

					if (event.key.code == sf::Keyboard::Key::A)
					{
						if (key == Key::KEY_STOP)
						{
							key = Key::KEY_LEFT;
							next_key = Key::KEY_STOP;
						}
						else
						{
							next_key = Key::KEY_LEFT;
						}
					}
					if (event.key.code == sf::Keyboard::Key::D)
					{
						if (key == Key::KEY_STOP)
						{
							key = Key::KEY_RIGHT;
							next_key = Key::KEY_STOP;
						}
						else
						{
							next_key = Key::KEY_RIGHT;
						}
					}
					if (event.key.code == sf::Keyboard::Key::W)
					{
						if (key == Key::KEY_STOP)
						{
							key = Key::KEY_UP;
							next_key = Key::KEY_STOP;
						}
						else
						{
							next_key = Key::KEY_UP;
						}
					}
					if (event.key.code == sf::Keyboard::Key::S)
					{
						if (key == Key::KEY_STOP)
						{
							key = Key::KEY_DOWN;
							next_key = Key::KEY_STOP;
						}
						else
						{
							next_key = Key::KEY_DOWN;
						}
					}
					if (event.key.code == sf::Keyboard::Key::R)
					{
						if (!game_over)
						{
							break;
						}

						game_over = false;
						level = 1;
						scatter_time = SCATTER_TIME;
						chase_time = CHASE_TIME;

						for (size_t i = 0; i < MAP_HEIGHT; i++)
						{
							for (size_t j = 0; j < MAP_WIDTH; j++)
							{
								if (map[i][j].item == Item_Type::COMMON || (map[i][j].item == Item_Type::ENERGIZER))
								{
									map[i][j].has_item = true;
								}
							}
						}

						for (size_t i = 0; i < ghosts.size(); i++)
						{
							ghosts[i]->revive();
						}

						if (current_points > highscore)
						{
							highscore = current_points;
						}

						pacman.restart();

						key = Key::KEY_STOP;
						break;
					}
				}
			}

			if (game_over || play_pacman_death_anim)
			{
				lag = std::chrono::microseconds(FRAME_DURATION - 1);
				break;
			}

			dt = clock.restart().asSeconds();
			second += dt;

			move_ghosts(dt);

			if (pacman.move(key, next_key, dt, map))
			{
				for (size_t i = 0; i < ghosts.size(); i++)
				{
					current_mode = Mode::SCATTER_ENERGIZER;
					ghosts[i]->switch_texture(current_mode);
				}

				energizer_clock.restart();
			}

			energizer_timer = energizer_clock.getElapsedTime().asSeconds();
			if (energizer_timer > ENERGIZER_TIME)
			{
				if (current_mode == Mode::SCATTER_ENERGIZER)
				{
					current_mode = Mode::SCATTER;
					for (size_t i = 0; i < ghosts.size(); i++)
					{
						ghosts[i]->switch_texture(current_mode);
					}
				}

				energizer_timer = 0;
			}

			timer = mode_clock.getElapsedTime().asSeconds();
			if ((timer > chase_time) && current_mode == Mode::CHASE)
			{
				for (size_t i = 0; i < ghosts.size(); i++)
				{
					ghosts[i]->update_pathfinding(map, corners[i]);
					ghosts[i]->switch_mode(Mode::SCATTER);
					reset_cells();
				}
				current_mode = Mode::SCATTER;

				mode_clock.restart();
				timer = 0;
			}
			else if(timer > scatter_time && current_mode == Mode::SCATTER)
			{
				for (size_t i = 0; i < ghosts.size(); i++)
				{
					ghosts[i]->switch_mode(Mode::CHASE);
				}
				current_mode = Mode::CHASE;

				mode_clock.restart();
				timer = 0;
			}

			set_path(key, corner, second);

			for (size_t i = 0; i < ghosts.size(); i++)
			{
				if (pacman.check_collision(ghosts[i]))
				{
					play_pacman_death_anim = true;
					break;
				}
			}

			level_up();
		}

		if (std::chrono::microseconds(FRAME_DURATION) > lag)
		{
			window.clear(sf::Color(0, 0, 0));
			draw_map();
			if (game_over) 
			{
				text.setPosition(((MAP_WIDTH/2)* CELL_SIZE) / 2, 15 * CELL_SIZE);
				text.setString("GAME OVER! Press 'R' to restart...");
				window.draw(text);
			}
			else
			{
				if (!play_pacman_death_anim)
				{
					pacman.draw(window);
					draw_ghosts();
				}
				else
				{
					dt = clock.restart().asSeconds();
					if (pacman.play_death_animation(dt, window))
					{
						play_pacman_death_anim = false;

						if (!pacman.get_lives())
						{
							game_over = true;
						}
						else
						{
							for (size_t i = 0; i < ghosts.size(); i++)
							{
								ghosts[i]->revive();
							}

							pacman.restart();
						}
					}
				}
			}

			text.setFillColor(sf::Color::Red);
			text.setPosition(0, (MAP_HEIGHT + 1)* CELL_SIZE + CELL_SIZE/2);
			text.setString("LIVES: " + std::to_string(pacman.get_lives()));
			window.draw(text);

			current_points = pacman.get_points();
			text.setFillColor(sf::Color::White);
			text.setPosition(0, 0);
			text.setString("1UP\t\tHIGH SCORE\t\tLEVEL: " + std::to_string(level) + "\n" + std::to_string(current_points) + "\t\t  " + std::to_string(current_points > highscore ? current_points : highscore));
			window.draw(text);

			window.display();
		}
	}
	
}

void Game::set_path(const Key& key, size_t& corner, float &second)
{
	if (second <= UPDATE_TIME)
	{
		return;
	}

	second = 0;

	sf::Vector2f target = pacman.get_position();
	sf::Vector2f delta;
	int floor = 6;
	for (size_t i = 0; i < ghosts.size(); i++)
	{
		switch (ghosts[i]->get_current_mode())
		{
		case Mode::SCATTER:
			if (corner == 4)
			{
				std::random_shuffle(std::begin(corners), std::end(corners));
			}
			if (ghosts[i]->get_target_state())
			{
				if (corners[i] == sf::Vector2f(ghosts[i]->get_position().x - 5, ghosts[i]->get_position().y - 5))
				{
					if (i)
					{
						std::swap(corners[0], corners[i]);
					}
					else
					{
						std::swap(corners[3], corners[i]);
					}
				}

				ghosts[i]->update_pathfinding(map, corners[i]);
				reset_cells();
				corner--;
			}
			if (!corner)
			{
				corner = 4;
			}
			break;
		case Mode::CHASE:
			switch (i)
			{
			case 0:
			{
				ghosts[0]->update_pathfinding(map, pacman.get_position());	//path for first ghost
				reset_cells();
				break;
			}
			case 1:
			{
				switch (key)
				{
				case Key::KEY_LEFT:
					delta.x = -floor * CELL_SIZE;
					delta.y = 0;
					break;
				case Key::KEY_RIGHT:
					delta.x = floor * CELL_SIZE;
					delta.y = 0;
					break;
				case Key::KEY_UP:
					delta.x = 0;
					delta.y = -floor * CELL_SIZE;
					break;
				case Key::KEY_DOWN:
					delta.x = 0;
					delta.y = floor * CELL_SIZE;
					break;
				default:
					delta.x = -floor * CELL_SIZE;
					delta.y = 0;
					break;
				}

				target += delta;
				while (!((target.x >= 0 && target.x < (MAP_WIDTH - 1) * CELL_SIZE) && (target.y >= 0 && target.y < (MAP_HEIGHT - 1) * CELL_SIZE)))
				{
					target = pacman.get_position();
					if (delta.x)
					{
						delta.x -= (delta.x > 0) ? (CELL_SIZE) : (-CELL_SIZE);
					}
					if (delta.y)
					{
						delta.y -= (delta.y > 0) ? (CELL_SIZE) : (-CELL_SIZE);
					}
					target += delta;
				}

				do
				{
					target = pacman.get_position();
					if (delta.x)
					{
						delta.x -= (delta.x > 0) ? (CELL_SIZE) : (-CELL_SIZE);
					}
					if (delta.y)
					{
						delta.y -= (delta.y > 0) ? (CELL_SIZE) : (-CELL_SIZE);
					}
					target += delta;

				} while (map[std::round(target.y / CELL_SIZE)][std::round(target.x / CELL_SIZE)].type == Cell_Type::WALL);

				ghosts[1]->update_pathfinding(map, target);	//path for second ghost
				reset_cells();
				break;
			}
			case 2:
			{
				target = pacman.get_position();
				target.x += target.x / 2 - ghosts[0]->get_position().x;
				target.y += target.y / 2 - ghosts[0]->get_position().y;

				if (!((target.x >= 0 && target.x < (MAP_WIDTH - 1) * CELL_SIZE) && (target.y >= 0 && target.y < (MAP_HEIGHT - 1) * CELL_SIZE)))
				{
					ghosts[2]->update_pathfinding(map, pacman.get_position());
					reset_cells();
					break;
				}

				if (map[std::round(target.y / CELL_SIZE)][std::round(target.x / CELL_SIZE)].type != Cell_Type::FLOOR)
				{
					ghosts[2]->update_pathfinding(map, pacman.get_position());
					reset_cells();
					break;
				}

				ghosts[2]->update_pathfinding(map, target); //path for third ghost
				reset_cells();
				break;
			}
			case 3:
			{
				if (ghosts[3]->distance_to(pacman.get_position()) < CELL_SIZE * 5)
				{
					ghosts[3]->switch_mode(Mode::SCATTER);
					break;
				}

				ghosts[3]->update_pathfinding(map, pacman.get_position()); //path for forth ghost
				reset_cells();
				break;
			}
			}
		default:
			break;
		}

	}
}

void Game::reset_cells()
{
	for (size_t i = 0; i < MAP_HEIGHT; i++)
	{
		for (size_t j = 0; j < MAP_WIDTH; j++)
		{
			map[i][j].parent = nullptr;
			map[i][j].H = 0;
			map[i][j].G = 0;
			map[i][j].F = 0;
		}
	}
}

void Game::draw_ghosts() 
{
	for (size_t i = 0; i < ghosts.size(); i++)
	{
		ghosts[i]->draw(window);
	}
}

void Game::move_ghosts(const float delta_time) 
{
	for (size_t i = 0; i < ghosts.size(); i++)
	{
		ghosts[i]->move(i, delta_time);
	}
}

void Game::draw_map() 
{
	sf::Sprite sprite;
	sprite.setTexture(items_texture);

	size_t cell = 0;
	for (size_t i = 0; i < MAP_HEIGHT; i++)
	{
		for (size_t j = 0; j < MAP_WIDTH; j++)
		{
			if (i == 11 && j == 11)
			{
				int y = 0;
			}

			if (map[i][j].type == Cell_Type::WALL)
			{
				if (i > 0)
				{
					if (map[i - 1][j].type == Cell_Type::WALL)
					{
						cell++;
					}
				}

				if (j < MAP_WIDTH - 1)
				{
					if (map[i][j + 1].type == Cell_Type::WALL)
					{
						cell += 2;
					}
				}

				if (i < MAP_HEIGHT - 1)
				{
					if (map[i + 1][j].type == Cell_Type::WALL)
					{
						cell += 4;
					}
				}

				if (j > 0)
				{
					if (map[i][j - 1].type == Cell_Type::WALL)
					{
						cell += 8;
					}
				}

				map[i][j].sprite.setTextureRect(sf::IntRect((cell) * 16, 0, 16, 16));
				map[i][j].sprite.setPosition(sf::Vector2f(j * CELL_SIZE + 2, i * CELL_SIZE + CELL_SIZE));
				map[i][j].sprite.setScale(2.285f, 2.285f);
				window.draw(map[i][j].sprite);

				cell = 0;
			}

			if (map[i][j].has_item)
			{
				if (map[i][j].item == Item_Type::COMMON)
				{
					sprite.setTextureRect(sf::IntRect(0, 0, 14, 14));
					sprite.setPosition(sf::Vector2f(j * CELL_SIZE + CELL_SIZE / 2 - 5, i * CELL_SIZE + CELL_SIZE / 2 + CELL_SIZE - 5));
				}
				else if (map[i][j].item == Item_Type::ENERGIZER)
				{
					sprite.setTextureRect(sf::IntRect(14, 0, 14, 14));
					sprite.setPosition(sf::Vector2f(j * CELL_SIZE + CELL_SIZE / 2 - 7, i * CELL_SIZE + CELL_SIZE / 2 + CELL_SIZE - 7));
				}
				sprite.setScale(1.0f, 1.0f);
				window.draw(sprite);
			}

			if (map[i][j].type == Cell_Type::DOOR)
			{
				sprite.setTextureRect(sf::IntRect(32, 0, 16, 16));
				sprite.setPosition(sf::Vector2f(j * CELL_SIZE + 2, i * CELL_SIZE + CELL_SIZE - CELL_SIZE/3));
				sprite.setScale(2.1f, 2.0f);
				window.draw(sprite);
			}
		}
	}
}

void Game::get_map() 
{
	sf::Image map_sketch;
	map_sketch.loadFromFile(MAP_PATH);

	sf::Vector2f position;
	sf::Color pixel;
	std::unique_ptr<Ghost> ghost;

	size_t itr = 0;
	for (unsigned char i = 0; i < MAP_HEIGHT; i++)
	{
		for (unsigned char j = 0; j < MAP_WIDTH; j++)
		{
			pixel = map_sketch.getPixel(j, i);
			if (pixel == sf::Color(0, 0, 0)) //wall
			{
				map[i][j].type = Cell_Type::WALL;
				map[i][j].item = Item_Type::EMPTY;
			}
			else if (pixel == sf::Color(255, 255, 255))	//floor(placeble)
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::COMMON;
				map[i][j].has_item = true;
			}
			else if (pixel == sf::Color(0, 255, 0))	//floor
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;
			}
			else if (pixel == sf::Color(255, 255, 0)) //pacman's starting position
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;

				position = sf::Vector2f(j * CELL_SIZE, i * CELL_SIZE);
				pacman.set_restart_position(position);
				pacman.set_position(position);
			}
			else if (pixel == sf::Color(255, 0, 0))	//ghost 1 starting position
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;

				position = sf::Vector2f(j * CELL_SIZE - 5, i * CELL_SIZE - 5);
				ghost = std::make_unique<Ghost>(position, current_mode, "ghosts_red.png", "ghosts_scared.png");
				ghosts.push_back(std::move(ghost));
			}
			else if (pixel == sf::Color(255, 0, 1))	//ghost 2 starting position
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;

				position = sf::Vector2f(j * CELL_SIZE - 5, i * CELL_SIZE - 5);
				ghost = std::make_unique<Ghost>(position, current_mode, "ghosts_pinky.png", "ghosts_scared.png");
				ghosts.push_back(std::move(ghost));
			}
			else if (pixel == sf::Color(255, 0, 2))	//ghost 3 starting position
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;

				position = sf::Vector2f(j * CELL_SIZE - 5, i * CELL_SIZE - 5);
				ghost = std::make_unique<Ghost>(position, current_mode, "ghosts_cyan.png", "ghosts_scared.png");
				ghosts.push_back(std::move(ghost));
			}
			else if (pixel == sf::Color(255, 0, 3))	//ghost 4 starting position
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;

				position = sf::Vector2f(j * CELL_SIZE - 5, i * CELL_SIZE - 5);
				ghost = std::make_unique<Ghost>(position, current_mode, "ghosts_yellow.png", "ghosts_scared.png");
				ghosts.push_back(std::move(ghost));
			}
			else if (pixel == sf::Color(0, 0, 255))	//energizer
			{
				map[i][j].type = Cell_Type::FLOOR;
				map[i][j].item = Item_Type::ENERGIZER;
				map[i][j].has_item = true;
			}
			else if (pixel == sf::Color(0, 255, 255))	//door
			{
				map[i][j].type = Cell_Type::DOOR;
				map[i][j].item = Item_Type::EMPTY;
				map[i][j].has_item = false;
			}

			map[i][j].sprite.setTexture(map_texture);
			map[i][j].column_index = j;
			map[i][j].row_index = i;
		}
	}
}

void Game::level_up() 
{
	for (size_t i = 0; i < MAP_HEIGHT; i++)
	{
		for (size_t j = 0; j < MAP_WIDTH; j++)
		{
			if (map[i][j].has_item)
			{
				return;
			}
		}
	}

	chase_time += 0.2;
	
	if (scatter_time > 2)
	{
		scatter_time -= 0.2;
	}

	for (size_t i = 0; i < MAP_HEIGHT; i++)
	{
		for (size_t j = 0; j < MAP_WIDTH; j++)
		{
			if (map[i][j].item == Item_Type::COMMON || (map[i][j].item == Item_Type::ENERGIZER))
			{
				map[i][j].has_item = true;
			}
		}
	}

	for (size_t i = 0; i < ghosts.size(); i++)
	{
		ghosts[i]->revive();
	}

	pacman.restart();

	level++;
}

