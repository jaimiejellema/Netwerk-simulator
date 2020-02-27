//
// Created by jaimie on 23-12-19.
//

#ifndef NETWORKSYSTEMS_GRAPHRENDERER_H
#define NETWORKSYSTEMS_GRAPHRENDERER_H

#include <map>
#include <SFML/Graphics.hpp>
#include "graph.h"

class GraphRenderer {
public:
    GraphRenderer(Graph* graph);

    void addCircle(short value);
    void removeCircle(short value);

    void renderWindow(sf::RenderWindow& window);
    void graphFromRoutes(std::unordered_map<ip_t, std::vector<ip_t>> routes);

private:
    int window_size_x;
    int window_size_y;


    std::map<uint16_t, sf::CircleShape*> NodeSprites;
    std::map<uint16_t, sf::Text*> NodeText;
    sf::Font font;

//Vanwege de manier waarop circle werken, doet een redraw alle circle objecten vernietigen en opnieuw instantieren
    bool circleRedrawRequired = false;

//Voolopig niet nodig vanwege lelijke rendering van lijnen momenteel
    bool connectionRedrawRequired = false;

    uint16_t currentClickedNode = 0;
    bool addLineActive = false;
    bool removeLineActive = false;

    sf::Vector2f oldPos;
    bool moving = false;
    float zoom = 1;

    Graph* nodeConnectionGraph;

    void addConnection(std::pair<uint16_t, sf::CircleShape*> sprite);
    void removeConnection(std::pair<uint16_t, sf::CircleShape*> sprite);
    void handleConnectionChange(sf::RenderWindow& window, sf::Event event);
    void handleEvent(sf::RenderWindow& window, sf::Event event);
};

#endif //NETWORKSYSTEMS_GRAPHRENDERER_H
