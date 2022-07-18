#pragma once
#include <SFML/Graphics.hpp>

class Animation
{
public:
	sf::IntRect rectangle;

	Animation(sf::Texture* texture, sf::Vector2u totalImage, float speed);
	Animation();

	bool update(size_t row, const float deltaTime);
	void set_frame(const size_t frame);
	unsigned int get_frame() const;
private:
	sf::Vector2u imageCount;
	sf::Vector2u currentImage;

	float totalTime;
	float switchTime;
};
