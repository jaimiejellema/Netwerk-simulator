//
// Created by jaimie on 23-12-19.
//

#include <iostream>
#include <cmath>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <set>
#include "graphRenderer.h"
/*
 * User has clicked on nodes, this code will change connection based on where was clicked
 * and what keys are pressed:
 * Hold CTRL to delete links
 * Hold SHIFT to add/remove unidirectionally instead of bidirectional.
 */

GraphRenderer::GraphRenderer(Graph* graph) : nodeConnectionGraph(graph) {
    font.loadFromFile("LiberationSans-Regular.ttf");
}

void GraphRenderer::addCircle(short value) {
    sf::CircleShape* shape = new sf::CircleShape(50.f);
    shape->setFillColor(sf::Color(150, 50, 250));
    shape->setOutlineThickness(10.f);
    shape->setOutlineColor(sf::Color(250, 150, 100));
    shape->setPosition(window_size_x / 2, window_size_y / 2);
    shape->setOrigin(50.0f, 50.0f);

    NodeSprites.insert({value, shape});


    sf::Text* text = new sf::Text();
    text->setString(std::to_string(value));
    text->setOrigin(text->getLocalBounds().width, text->getLocalBounds().height);
    text->setFillColor(sf::Color::Blue);
    text->setCharacterSize(50);
    text->setFont(font);

    NodeText.insert({value, text});

    circleRedrawRequired = true;
    connectionRedrawRequired = true;
}

void GraphRenderer::removeCircle(short value) {
    delete NodeSprites[value];
    delete NodeText[value];

    NodeSprites.erase(value);
    NodeText.erase(value);

    circleRedrawRequired = true;
    connectionRedrawRequired = true;
}

void GraphRenderer::addConnection(std::pair<uint16_t, sf::CircleShape*> sprite) {
    if (addLineActive) {
        addLineActive = false;

        //if same is node selected, cannot make a connection
        if (currentClickedNode != sprite.first) {
            nodeConnectionGraph->addEdge(sprite.first, currentClickedNode);
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                nodeConnectionGraph->addEdge(currentClickedNode, sprite.first);
            }
        }

        sf::CircleShape* node = NodeSprites[currentClickedNode];

        node->setFillColor(sf::Color(150, 50, 250));
        node->setOutlineColor(sf::Color(250, 150, 100));
    } else {
        addLineActive = true;
        currentClickedNode = sprite.first;
        sprite.second->setFillColor(sf::Color(250, 150, 100));
        sprite.second->setOutlineColor(sf::Color(150, 50, 250));
    }
}

void GraphRenderer::removeConnection(std::pair<uint16_t, sf::CircleShape*> sprite) {
    if (removeLineActive) {
        removeLineActive = false;

        if (currentClickedNode != sprite.first) {
            nodeConnectionGraph->removeEdge(sprite.first, currentClickedNode);
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                nodeConnectionGraph->removeEdge(currentClickedNode, sprite.first);
            }
        }
        sf::CircleShape* node = NodeSprites[currentClickedNode];

        node->setFillColor(sf::Color(150, 50, 250));
        node->setOutlineColor(sf::Color(250, 150, 100));
    } else {
        removeLineActive = true;
        currentClickedNode = sprite.first;
        sprite.second->setFillColor(sf::Color(250, 150, 100));
        sprite.second->setOutlineColor(sf::Color(150, 50, 250));
    }
}


void GraphRenderer::handleConnectionChange(sf::RenderWindow& window, sf::Event event) {
    for(auto& sprite : NodeSprites) {
        float r = sprite.second->getRadius();
        sf::Vector2f pos = sprite.second->getPosition() - window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        if ((pos.x * pos.x + pos.y * pos.y) < (r * r)) {
            std::cout << "Node was clicked " << sprite.first << std::endl;
            circleRedrawRequired = true;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                removeConnection(sprite);
            } else {
                addConnection(sprite);
            }
        }
    }
}

void GraphRenderer::handleEvent(sf::RenderWindow& window, sf::Event event) {

    //Left mouse button to just change position of the current object.
    //Right mouse button, to move the complete viewport
    //Scrolling changes the size of everything
    sf::View view = window.getView();

    switch (event.type) {
        case sf::Event::Closed:
            window.close();
            break;
        case sf::Event::Resized:
            window_size_x = event.size.width;
            window_size_y = event.size.height;
            circleRedrawRequired = true;
            break;
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            }

            if (event.key.code == sf::Keyboard::Q) {
                //Cancel active selections if Q is pressed
                addLineActive = false;
                removeLineActive = false;
                currentClickedNode = 0;
                circleRedrawRequired = true;
                sf::CircleShape* node = NodeSprites[currentClickedNode];

                node->setFillColor(sf::Color(150, 50, 250));
                node->setOutlineColor(sf::Color(250, 150, 100));
            }

            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left) {
                handleConnectionChange(window, event);
            }

            // Mouse button is pressed, moving background, get the position and set moving as active
            if (event.mouseButton.button == sf::Mouse::Right) {
                moving = true;
                oldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            }
            break;
        case  sf::Event::MouseButtonReleased:
            // Mouse button is released, no longer move
            if (event.mouseButton.button == sf::Mouse::Right) {
                moving = false;
            }
            break;
        case sf::Event::MouseMoved:
        {
            // Ignore mouse movement unless a button is pressed (see above)
            if (!moving)
                break;

            // Determine the new position in world coordinates
            const sf::Vector2f newPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));

            // Determine how the cursor has moved
            const sf::Vector2f deltaPos = oldPos - newPos;

            // Move our view accordingly and update the window
            view.setCenter(view.getCenter() + deltaPos);
            window.setView(view);

            oldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            break;
        }
        case sf::Event::MouseWheelScrolled:
            // Ignore the mouse wheel unless we're not moving
            if (moving)
                break;

            // Determine the scroll direction and adjust the zoom level
            if (event.mouseWheelScroll.delta <= -1)
                zoom = std::min(2.f, zoom + .1f);
            else if (event.mouseWheelScroll.delta >= 1)
                zoom = std::max(.5f, zoom - .1f);

            // Update our view
            view.setSize(window.getDefaultView().getSize());
            view.zoom(zoom);
            window.setView(view);
            break;
        default:
            break;
    }
}

void GraphRenderer::graphFromRoutes(std::unordered_map<ip_t, std::vector<ip_t>> routes) {
    NodeText.clear();
    NodeSprites.clear();
    nodeConnectionGraph->clear();

    std::set<ip_t> existingNodes;
    for (auto& neighbours : routes) {
        for (auto& node : neighbours.second) {
            existingNodes.insert(node);
        }
        existingNodes.insert(neighbours.first);
    }

    for (auto& node: existingNodes) {
        addCircle(node);
    }

    nodeConnectionGraph->update(routes, UINT16_MAX);

    circleRedrawRequired = true;
    connectionRedrawRequired = true;

}

void GraphRenderer::renderWindow(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        handleEvent(window, event);
    }

    // Draw our simple scene
    window.clear(sf::Color::White);

    //All nodes are put on a circle in order with equal spacing
    //The connections are drawn as needed between the nodes
    if (circleRedrawRequired) {
        circleRedrawRequired = false;
        int center_x = window_size_x / 2;
        int center_y = window_size_x / 2;

        //rod_length is the radius of the circle on which the nodes are drawn
        int rod_length = std::min(window_size_x, window_size_y) / 3;
        float angle = (2 * M_PI) / NodeSprites.size();

        int i = 0;
        for(auto& sprite : NodeSprites) {
            sprite.second->setPosition(center_x + (rod_length * sin(angle * i)), center_y - rod_length * cos(angle * i));
            i++;
        }

        i = 0;
        for(auto& sprite : NodeText) {
            sprite.second->setPosition(center_x + (rod_length * sin(angle * i)), center_y - rod_length * cos(angle * i));
            //sprite.second->setPosition(center_x,center_y);
            i++;
        }
    }


    for(auto& sprite : NodeSprites) {
        window.draw(*sprite.second);
        std::vector<short> neighbors = nodeConnectionGraph->getNeighbors(sprite.first);

        //Draw a line between all nodes that are connected in the graph
        for (auto node : neighbors) {
            if (NodeSprites.find(node) != NodeSprites.end()) {
                //Calculate position of the line
                sf::Vector2f vec = sprite.second->getPosition() - NodeSprites[node]->getPosition();
                sf::Vector2f center = (sprite.second->getPosition() + NodeSprites[node]->getPosition()) / 2.0f;

                //Offset lines to show directionality
                sf::Vector2f offset(-vec.y, vec.x);
                float scale = 1 * sqrt(offset.x * offset.x + offset.y * offset.y);
                offset /= scale;
                offset = sf::Vector2f(20 * offset.x, 20 * offset.y);

                //Create line
                sf::RectangleShape *rect = new sf::RectangleShape(sf::Vector2f(scale, 10)); //scale is probably not the right choice here since it's affected by scaling factor
                rect->setFillColor(sf::Color(150, 50, 250));
                rect->setPosition(center + offset);
                rect->setRotation(std::atan2(vec.y, vec.x) / M_PI * 180);
                rect->setOrigin(scale / 2, 5);
                window.draw(*rect);

                delete rect;
            }
        }

    }

    for(auto& sprite : NodeText) {
        window.draw(*sprite.second);
    }

    window.display();
}
