#ifndef NODE_HPP
#define NODE_HPP

#include "SFML/Graphics.hpp"
#include <string>
#include <vector>

class Node
{
public:
  Node(sf::Vector2f position, sf::String message, sf::Font* font,
         int size, sf::Color color);
  bool Update(sf::Vector2i mousePos, bool isClicking); //update button status
  void Draw(sf::RenderWindow* window);
  std::string Type;
  std::string Name;
  std::string Command;
  bool Processed;
  std::vector<Node*> Children;
  std::vector<Node*> Parents;
  sf::Text Message; //message to display in button
private:
  bool prevMouseState; //previous left click state
  sf::Vector2i prevMousePos; //previous mouse position
  bool Clicked;
};

#endif
