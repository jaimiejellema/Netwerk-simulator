//
// Created by jaimie on 2018-09-19.
//


#include "connectivity.h"

#include <mutex>
#include <thread>
#include <SFML/Graphics/RenderWindow.hpp>

#include "packet.h"
#include "pingPacket.h"
#include "ringBuffer.h"
#include "routeInfoPacket.h"
#include "graphRenderer.h"

std::mutex mutex_graph;
GraphRenderer graphrenderer = NULL;

Connectivity::Connectivity(ip_t address) : client_address(address) {

}

std::vector<ip_t> Connectivity::get_connection_status() {
    return connection_status;
}


void Connectivity::update() {
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    connection_status.clear();

    //After time-out delete node and corresponding routing info for nodes which are no longer present in network
    for (auto r = route_time.cbegin(); r != route_time.cend();) {
        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds >(
                currentTime - r->second
        );

        if (duration.count() > FORGET_NODE_TIMEOUT_MS) {
            //comm->logFile << "Removing " << r->first << " as possible route" << std::endl;
            routes.erase(r->first);
            r = route_time.erase(r);
        } else {
            r++;
        }
    }

    //Determine if enough ping beacons where received to consider this link working.
    //Allowing for some amount of packet drop
    for (auto &x : pings_received) {
        int counter = 0;
        for (int i = 0; i < PING_TRIES; i++){
            std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds >(
                    currentTime - x.second.read(i)
            );

            if (duration.count() < PING_TIMEOUT_MS * (PING_TRIES + 0.5)) {
                counter++;
            }
        }

        if (counter > PING_TRIES/2) {
            //comm->logFile << "Adding " << x.first << " as possible route" << std::endl;
            connection_status.push_back(x.first);
        }
    }

    //TODO: graphrenderer
    //graphrenderer.graphFromRoutes(routes);
    routing.update(routes, client_address);
}


void Connectivity::pingReceived(ip_t address) {
    mutex_graph.lock();
    std::unordered_map<ip_t, RingBuffer>::iterator it = pings_received.find(address);
    if (it != pings_received.end()) {
        it->second.write(std::chrono::system_clock::now());
    } else {
        RingBuffer buf;
        buf.write(std::chrono::system_clock::now());
        pings_received.insert({address, buf});
    }
    update();
    mutex_graph.unlock();
}

void Connectivity::routeReceived(ip_t address, std::vector<ip_t> reachable) {
    mutex_graph.lock();

    //Only update the graph if the routing data is different then it was, aka the network changed
    if (routes[address] != reachable) {
        routes[address] = reachable;

        routing.update(routes, client_address);
        //TODO: graphrenderer
        //graphrenderer.graphFromRoutes(routes);

    }

    route_time[address] = std::chrono::system_clock::now();
    mutex_graph.unlock();
};

ip_t Connectivity::getNextHop(ip_t destination) {
    return routing.getNextHop(destination);
}

void Connectivity::sendPingPackets(Communicator *comm) {
    mutex_graph.lock();
    packet_header h;
    h.source_address = comm->getAddress();
    h.proxy_address = BROADCAST_ADDRESS;
    h.destination_address = BROADCAST_ADDRESS;
    h.payload_size = 0;
    h.payload_type = PING;
    h.sender_nonce = comm->getNonce();
    std::unique_ptr<Packet> ptr = std::unique_ptr<Packet>(new PingPacket(comm, h));
    comm->sendPacket(ptr.get());
    update();

    mutex_graph.unlock();
}

void Connectivity::sendRoutePackets(Communicator *comm) {
    packet_header h;
    h.source_address = comm->getAddress();
    h.proxy_address = BROADCAST_ADDRESS;
    h.destination_address = BROADCAST_ADDRESS;

    h.payload_size = get_connection_status().size() * sizeof(ip_t);
    h.payload_type = ROUTING_DATA;
    h.sender_nonce = comm->getNonce();
    std::unique_ptr<Packet> ptr = std::unique_ptr<Packet>(new RouteInfoPacket(comm, h, get_connection_status()));
    comm->sendPacket(ptr.get());
}

void Connectivity::connectionRenderLoop() {
    //TODO: graphrenderer
    /*
    sf::RenderWindow window(sf::VideoMode(400, 200), "Client connection graph");

    Graph nodeConnectionGraph = Graph();
    graphrenderer = GraphRenderer(&nodeConnectionGraph);


    // Retrieve the window's default view
    sf::View view = window.getDefaultView();


    while (window.isOpen()) {
        mutex_graph.lock();
        graphrenderer.renderWindow(window);
        mutex_graph.unlock();
    }
    */
}

void Connectivity::renderThread() {
    connectionRenderLoop();
}