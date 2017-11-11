#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "SFML/Graphics.hpp"

class Button
{
public:
  Button(sf::Vector2f position, sf::String message, sf::Font* font,
         int size, sf::Color color, void (*action)());
  void Update(sf::Vector2i mousePos, bool isClicking); //update button status
  void Draw(sf::RenderWindow* window);
private:
  bool prevMouseState;
  bool Clicked;
  sf::Text Message; //message to display in button
  void (*Action)();
};

#endif
