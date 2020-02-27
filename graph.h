//
// Created by jaimie on 2018-09-19.
//

#ifndef NETWORKSYSTEMS_GRAPH_H
#define NETWORKSYSTEMS_GRAPH_H

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "constants.h"

//TODO: convert all short to ip_t?
/** Graph is used by both server and client to keep track of the possible connections between nodes
 *
 */
class Graph {
public:
    Graph();
    void clear();

    void addEdge(short from, short to);
    void removeEdge(short from, short to);
    std::vector<short> getNeighbors(short node);

    void update(std::unordered_map<ip_t, std::vector<ip_t>> map, ip_t client_address);
    ip_t getNextHop(ip_t destination);

    void printGraph();

private:
    int getCost(short u, short v);
    void bellmanFord(ip_t client_address);
    std::unordered_map<short, std::vector<short>> adjacency_matrix;
    std::unordered_map<short, short> direction;
};


#endif //NETWORKSYSTEMS_GRAPH_H
