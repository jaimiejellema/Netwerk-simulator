//
// Created by Jaimie on 2018-07-22.
// Modified from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include <thread>
#include <future>

#include "constants.h"
#include "graph.h"
#include "graphRenderer.h"

#include <SFML/Graphics.hpp>
#include <mutex>
#include <netinet/tcp.h>


const int DEFAULT_WINDOW_SIZE_X = 640;
const int DEFAULT_WINDOW_SIZE_Y = 480;

//TODO: race conditions....
std::mutex mutex_NodeSprites;           // mutex for critical section
GraphRenderer graphrenderer = NULL;

Graph nodeConnectionGraph;


void run_sockets(std::future<void> futureObj) {
    int opt = true;
    int client_socket[SIMULATOR_MAX_CLIENTS];
    ssize_t valread;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //lookup from FD to client id
    std::unordered_map<int, short> lookup;
    std::unordered_map<short, int> reverse_lookup;

    //Adjacency matrix of connected clients
    /*
    nodeConnectionGraph.addEdge(1, 2);
    nodeConnectionGraph.addEdge(2, 3);
    nodeConnectionGraph.addEdge(3,4);
    //nodeConnectionGraph.addEdge(4,1);

    nodeConnectionGraph.addEdge(2, 1);
    nodeConnectionGraph.addEdge(3, 2);
    nodeConnectionGraph.addEdge(4,3);
    //nodeConnectionGraph.addEdge(1,4);
    */

    srand(time(0));



    nodeConnectionGraph.addEdge(1, 2);
    nodeConnectionGraph.addEdge(2, 3);

    nodeConnectionGraph.addEdge(2, 1);
    nodeConnectionGraph.addEdge(3, 2);


    //set of socket descriptors
    fd_set readfds;

    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < SIMULATOR_MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    //create a master socket
    int master_socket;
    if( (master_socket = socket(AF_INET , SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //Allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    int flag = 1;
    if( setsockopt(master_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) ) {
        perror("setsockopt 2");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( SIMULATOR_PORT_NUMBER );

    //bind the socket
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", SIMULATOR_PORT_NUMBER);

    //Maximum of 3 pending connections
    if (listen(master_socket, 3) < 0) {
        perror("Failed to listen to port");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    int addrlen = sizeof(address);
    puts("Waiting for connections");

    while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        //add child sockets to set
        int sd;
        for (int i = 0; i < SIMULATOR_MAX_CLIENTS; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            //highest file descriptor number, need it for the select function
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        //Wait on the sockets with a 5 milliseconds timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;
        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        //incoming connection
        if (FD_ISSET(master_socket, &readfds)) {

            //Create new socket so master socket is free again
            int new_socket;
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
                perror("Error accepting new connection");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket,
                   inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            //keep array of sockets
            for (int i = 0; i < SIMULATOR_MAX_CLIENTS; i++) {
                //if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        //Check clients for I/O activity
        for (int i = 0; i < SIMULATOR_MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {

                //Check if it was for closing
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    //Somebody disconnected, get his details and print
                    getpeername(sd, (struct sockaddr *) &address, \
                            (socklen_t *) &addrlen);
                    printf("Host disconnected, ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //remove from lookup tables
                    mutex_NodeSprites.lock();
                    short value = lookup[sd];
                    graphrenderer.removeCircle(value);
                    mutex_NodeSprites.unlock();

                    reverse_lookup.erase(lookup[sd]);
                    lookup.erase(sd);

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                } else if (valread == -1) {
                    std::cout << "Error while reading data: " << errno << " " << strerror(errno) << std::endl;
                } else {
                    // Incoming IO
                    buffer[valread] = '\0';

                    //first message a client sends should be it's client_id, after which it's placed in the lookup table
                    if (lookup.find(sd) == lookup.end()) {
                        //TODO: this code might execute multiple times for each node that joins, so that might create bugs
                        short value = buffer[0] << 8 | buffer[1];
                        printf("Adding lookup %d -> %d\n", sd, value);
                        lookup.insert({sd, value});
                        reverse_lookup.insert({value, sd});

                        mutex_NodeSprites.lock();
                        graphrenderer.addCircle(value);
                        mutex_NodeSprites.unlock();
                    } else {
                        int id = lookup[sd];
                        //comm->logFile << "Sending replies " << id << std::endl;
                        for (auto const &value: nodeConnectionGraph.getNeighbors(id)) {
                            if (reverse_lookup.find(value) != reverse_lookup.end()) {
                                if (rand() % 100 < (100 - SERVER_DROP_CHANGE_PERCENT)) {
                                    printf("sending message of length %ld, %d -> %d\n", valread, id, value);
                                    send(reverse_lookup[value], buffer, valread, 0);
                                } else {
                                    printf("Dropped message of length %ld, %d -> %d\n", valread, id, value);
                                }
                            } else {
                                //printf("failed to send: %d not connected.\n", value);
                            }
                        }
                    }
                }
            }
        }
    }
}



int main(int argc , char *argv[]) {
    sf::RenderWindow window(sf::VideoMode(DEFAULT_WINDOW_SIZE_X, DEFAULT_WINDOW_SIZE_Y), "Server controller");

    nodeConnectionGraph = Graph();
    graphrenderer = GraphRenderer(&nodeConnectionGraph);


    // Retrieve the window's default view
    sf::View view = window.getDefaultView();


    // Create a std::promise object
    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();
    std::thread socket_runner(run_sockets, std::move(futureObj));

    while (window.isOpen()) {
        mutex_NodeSprites.lock();
        graphrenderer.renderWindow(window);
        mutex_NodeSprites.unlock();
    }


    exitSignal.set_value();
    socket_runner.join();
    return 0;
}