//
// Created by jaimie on 2018-09-19.
//

#include <iostream>
#include <SFML/Graphics.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <thread>




int main() {
    sf::RenderWindow window(sf::VideoMode(640, 480), "Server controller");

    // Create 2 basic reference shapes
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color(150, 50, 250));
    shape.setOutlineThickness(10.f);
    shape.setOutlineColor(sf::Color(250, 150, 100));
    shape.setPosition(320, 240);

    sf::CircleShape shape2(50.f);
    shape2.setFillColor(sf::Color(250, 150, 150));
    shape2.setOutlineThickness(10.f);
    shape2.setOutlineColor(sf::Color(50, 50, 20));
    shape2.setPosition(200, 50);

    sf::Vector2f oldPos;
    bool moving = false;

    float zoom = 1;

    // Retrieve the window's default view
    sf::View view = window.getDefaultView();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {

            //Left mouse button to just change position of the current object.
            //Right mouse button, to move the complete viewport
            //Scrolling changes the size of everything

            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    // Mouse button is pressed, get the position and set moving as active
                    if (event.mouseButton.button == 1) {
                        moving = true;
                        oldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    }
                    break;
                case  sf::Event::MouseButtonReleased:
                    // Mouse button is released, no longer move
                    if (event.mouseButton.button == 1) {
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
                        // Swap these to invert the movement direction
                        const sf::Vector2f deltaPos = oldPos - newPos;

                        // Move our view accordingly and update the window
                        view.setCenter(view.getCenter() + deltaPos);
                        window.setView(view);

                        //Calculate the new old position in the current coordinate system
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
            }
        }

        // Draw our simple scene
        window.clear(sf::Color::White);
        window.draw(shape);
        window.draw(shape2);
        window.display();
    }
}