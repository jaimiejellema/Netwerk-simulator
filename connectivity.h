//
// Created by jaimie on 2018-09-19.
//

#ifndef NETWORKSYSTEMS_CONNECTIVITY_H
#define NETWORKSYSTEMS_CONNECTIVITY_H

#include "constants.h"
#include "communicator.h"
#include "ringBuffer.h"
#include "graph.h"
#include "graphRenderer.h"
#include <chrono>
#include <unordered_map>
#include <thread>

/** Keeps track of the network graph that the P2P clients have
 * This is a 3 step process.
 * 1) Send ping packets/beacons to determine neighborhood.
 * NOTE: this ping packets are beacons in the sense that no response is expected when receiving a ping packet.
 *
 * 2) Based on received ping beacons, flood network with table of nodes that can reach you.
 * 3) Use all received tables to build a directional graph of the network
 */

class Connectivity {
public:
    Connectivity(ip_t origin);
    void pingReceived(ip_t address);
    void routeReceived(ip_t origin, std::vector<ip_t> reachable);
    void sendPingPackets(Communicator *comm);
    void sendRoutePackets(Communicator *comm);

    /** Use graph to determine what next hop is order to reach destination */
    ip_t getNextHop(ip_t destination);

    /** Based on received ping beacons update and route table update graph*/
    void update();
    std::vector<ip_t> get_connection_status();
    static void renderThread();

private:
    static void connectionRenderLoop();

    ip_t client_address;

    std::chrono::time_point<std::chrono::system_clock> last_ping_send;

    //ring buffer of last pings received per ip
    std::unordered_map<ip_t, RingBuffer> pings_received;

    //list of all nodes I believe I have direct connection to based on pings
    std::vector<ip_t> connection_status;

    //2D array of all possible node connections
    std::unordered_map<ip_t, std::vector<ip_t>> routes;

    //List of when I got the node connections, used to invalidate routes, if they are too old
    std::unordered_map<ip_t, std::chrono::time_point<std::chrono::system_clock>> route_time;

    Graph routing;
};


#endif //NETWORKSYSTEMS_CONNECTIVITY_H
