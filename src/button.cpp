#include "button.hpp"

Button::Button(sf::Vector2f position, sf::String message, sf::Font* font,
               int size, sf::Color color, void (*action)())
{
  Action = action;
  prevMouseState = false;
  Clicked = false;
  Message.setString(message);
  Message.setFont(*font);
  Message.setStyle(sf::Text::Regular);
  Message.setFillColor(color);
  Message.setPosition(position.x + 5, position.y + 5);
  Message.setCharacterSize(size);
}

void Button::Update(sf::Vector2i mousePos, bool isClicking)
{
  sf::Rect<float> rect = Message.getGlobalBounds();
  sf::RectangleShape rectangle;
  rectangle.setPosition(rect.left - 25, rect.top - 10);
  rectangle.setSize(sf::Vector2f(rect.width + 50, rect.height + 20));
  if (rectangle.getGlobalBounds().contains(mousePos.x, mousePos.y))
  {
    Clicked = isClicking;
    if (prevMouseState && !Clicked)
      (*Action)();
  }
  else
    Clicked = false;
  prevMouseState = Clicked;
}

void Button::Draw(sf::RenderWindow* window)
{
  sf::Rect<float> rect = Message.getGlobalBounds();
  sf::RectangleShape rectangle;
  if (!Clicked) rectangle.setOutlineColor(sf::Color::Black);
  else rectangle.setOutlineColor(sf::Color::Red);
  rectangle.setOutlineThickness(3);
  rectangle.setPosition(rect.left - 25, rect.top - 10);
  rectangle.setSize(sf::Vector2f(rect.width + 50, rect.height + 20));
  window->draw(rectangle);
  window->draw(Message);
}
