#include "node.hpp"

Node::Node(sf::Vector2f position, sf::String message, sf::Font* font,
               int size, sf::Color color)
{
  prevMouseState = false;
  Clicked = false;
  Message.setString(message);
  Message.setFont(*font);
  Message.setStyle(sf::Text::Regular);
  Message.setFillColor(color);
  Message.setPosition(position.x + 5, position.y + 5);
  Message.setCharacterSize(size);
  Processed = false;
}

bool Node::Update(sf::Vector2i mousePos, bool isClicking)
{
  bool activated = false;
  sf::Rect<float> rect = Message.getGlobalBounds();
  sf::RectangleShape rectangle;
  rectangle.setPosition(rect.left - 25, rect.top - 10);
  rectangle.setSize(sf::Vector2f(rect.width + 50, rect.height + 20));
  if (rectangle.getGlobalBounds().contains(mousePos.x, mousePos.y))
  {
    Clicked = isClicking;
    if (prevMouseState && !Clicked)
      activated = true;
  }
  else
    Clicked = false;

  if (prevMouseState && isClicking)
  {
    Message.move(mousePos.x - prevMousePos.x, mousePos.y - prevMousePos.y);
    prevMouseState = isClicking;
    prevMousePos = mousePos;
  }
  else
    prevMouseState = Clicked;
  prevMousePos = mousePos;
  return activated;
}

void Node::Draw(sf::RenderWindow* window)
{
  sf::Rect<float> rect = Message.getGlobalBounds();
  sf::RectangleShape rectangle;
  rectangle.setOutlineColor(sf::Color::Black);
  rectangle.setOutlineThickness(3);
  rectangle.setPosition(rect.left - 25, rect.top - 10);
  rectangle.setSize(sf::Vector2f(rect.width + 50, rect.height + 20));
  window->draw(rectangle);
  window->draw(Message);
}
