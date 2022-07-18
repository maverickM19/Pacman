#include "Animation.h" 

Animation::Animation(sf::Texture* texture, sf::Vector2u totalImage, float speed) : switchTime(speed), imageCount(totalImage), totalTime(0.0f)
{
	currentImage.x = 0;
	rectangle.width = std::round(texture->getSize().x / static_cast<float>(totalImage.x));
	rectangle.height = std::round(texture->getSize().y / static_cast<float>(totalImage.y));

	rectangle.left = 0 * rectangle.width;
	rectangle.top = 0 * rectangle.height;
}

void Animation::set_frame(const size_t frame) 
{
	currentImage.x = frame;
}

unsigned int Animation::get_frame() const 
{
	return currentImage.x;
}

Animation::Animation()
{
}

bool Animation::update(size_t row, const float deltaTime)
{
	currentImage.y = row;
	totalTime += deltaTime;

	if (totalTime >= switchTime)
	{
		totalTime -= switchTime;
		currentImage.x++;
	}

	if (currentImage.x == imageCount.x)
	{
		currentImage.x = 0;
	}

	rectangle.left = currentImage.x * rectangle.width;
	rectangle.top = currentImage.y * rectangle.height;

	return false;
}
