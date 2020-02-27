//
// Created by jaimie on 2018-09-19.
//

#include "graph.h"

#include <iostream>


Graph::Graph(){

}


void Graph::clear() {
    adjacency_matrix.clear();
}

int Graph::getCost(short from, short to) {
    //TODO: dynamic link cost. (Prefer bidirectional links over single directional links)
    return 1;
}

/** Basic Bellman-Ford algorithm to determine shortest path
 * It determines to direction to send packets coming from origin to every other node in the network
 *
 * It works using 3 steps,
 * 1) all nodes have infinite distance from me
 * 2) Loop over all nodes and see if the neighbor + link cost is faster then current path
 * This looping continues each time lowering the cost of reaching nodes.
 * While keeping track of which predecessor node we came from
 *
 * 3) Use predecessor information to determine from origin how to reach each node
 * @param origin what node are we, and hence will be sending all the packets
 */

void Graph::bellmanFord(ip_t client_address) {
    std::unordered_map<short, int> distance;
    std::unordered_map<short, int> predecessor;

    //No point in trying to pathfind if there are no known routes
    if (adjacency_matrix.size() == 0) {
        return;
    }

    distance.reserve(adjacency_matrix.size());
    predecessor.reserve(adjacency_matrix.size());

    //Note this only loops over all nodes which have outgoing connections
    //Which is a superset of all nodes you can possibly reach.
    for (auto it = adjacency_matrix.begin(); it != adjacency_matrix.end(); it++) {
        distance[it->first] = MAX_COST_PATH;
        predecessor[it->first] = 0;
    }

    distance[client_address] = 0;
    //TODO: is adjacency matrix size really the same size as the amount of edges I can possibly route
    for (unsigned long i = 0; i < (adjacency_matrix.size() - 1); i++) {
        //Iterating over all edges
        for (auto it = adjacency_matrix.begin(); it != adjacency_matrix.end(); it++) {
            for (short v : it->second) {
                int cost = getCost(it->first, v);
                if (distance[it->first] + cost < distance[v]) {
                    distance[v] = distance[it->first] + cost;
                    predecessor[v] = it->first;
                }
            }
        }
    }

    //No need to check negative weight cycle, since getCost is never negative
    int counter = 0;


    //For all nodes determine to which intermediate nodes it should send packets to arrive at destination
    //Basically flip the predecessor array you got from bellman ford.
    //TODO: this might bork on unidirectional links, and adjacency matrix is to from
    //So all possible destinations might not be in the adjaceny matrix index, which (may??) only contains "from" nodes

    for (auto it = adjacency_matrix.begin(); it != adjacency_matrix.end(); it++) {
        short node = it->first;
        if (distance[node] < MAX_COST_PATH && node != client_address) {
            while (predecessor[node] != client_address) {
                node = predecessor[node];
                counter++;
                if (counter > 100) {
                    std::cerr << "Broken for " << it->first << " " << node << " " << predecessor[node];
                    break;
                }
            }
            direction[it->first] = node;
        }
    }
}


void Graph::update(std::unordered_map<ip_t, std::vector<ip_t>> map, ip_t client_address) {
    clear();

    //Each nodes route-info contains a list of routes, which are ping packets he node received.
    //Hence if node A receives a ping from B, then B can transmit to A
    for (auto it : map) {
        for (const auto &to : it.second) {
            addEdge(to, it.first);
        }
    }

    if (client_address != UINT16_MAX) {
        bellmanFord(client_address);
    }
}

ip_t Graph::getNextHop(ip_t destination) {
    return (ip_t) direction[destination];
}

void Graph::addEdge(short from, short to) {
    if (adjacency_matrix.find(from) != adjacency_matrix.end()) {
        adjacency_matrix[from].push_back(to);
    } else {
        adjacency_matrix[from] = std::vector<short>();
        adjacency_matrix[from].push_back(to);
    }
}

void Graph::removeEdge(short from, short to) {
    adjacency_matrix[from].erase(std::remove(adjacency_matrix[from].begin(), adjacency_matrix[from].end(), to), adjacency_matrix[from].end());
}

std::vector<short> Graph::getNeighbors(short node) {
    if (adjacency_matrix.find(node) == adjacency_matrix.end()){
        return std::vector<short>();
    }

    return adjacency_matrix[node];
}


void Graph::printGraph() {
    std::cout << "Connection graph: " << std::endl;
    for (auto& it :  adjacency_matrix){
        std::cout << "Row " << it.first << std::endl;
        for (unsigned long j = 0; j < it.second.size(); j++){
            std::cout << it.second[j] << " ";
        }
        std::cout << std::endl;
    }
}