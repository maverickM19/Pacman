#pragma once
#include <SFML/Graphics.hpp>

const unsigned short SCREEN_WIDTH = 805;
const unsigned short SCREEN_HEIGHT = 992;

const unsigned short CELL_SIZE = 32;

const unsigned short MAP_WIDTH = 25;
const unsigned short MAP_HEIGHT = 29;

const unsigned int FRAME_DURATION = 16667;	//	1 / 60 = 16.67ms
const float ANIMATION_SPEED = 0.01f;

const char MAP_TEXTURE_PATH[] = "map_texture.png";
const char MAP_PATH[] = "map_remake.png";

const unsigned short ENERGIZER_TIME = 7;
const unsigned short CHASE_TIME = 7;
const unsigned short SCATTER_TIME = 7;

const float UPDATE_TIME = 0.5;

enum class Cell_Type
{
	WALL,
	FLOOR,
	DOOR
};

enum class Mode 
{
	SCATTER,
	CHASE,
	SCATTER_ENERGIZER
};

enum class Item_Type 
{
	COMMON,
	ENERGIZER,
	EMPTY
};

enum class Key
{
	KEY_RIGHT,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_STOP
};

struct Cell
{
	Cell_Type type;
	Item_Type item;
	sf::Sprite sprite;

	bool has_item;

	size_t column_index;
	size_t row_index;

	short H; //Heuristic / movement cost to goal
	short G; //Movement cost. (Total of entire path)
	short F; //Estimated cost for full path (G + H)

	Cell* parent;
};
