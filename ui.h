//
// Created by jaimie on 2018-10-30.
//

#ifndef NETWORKSYSTEMS_UI_H
#define NETWORKSYSTEMS_UI_H

#include <pthread.h>
#include "communicator.h"


class Ui {
private:
    Communicator* comm;

    enum client_state {LOBBY, IN_CHAT};
    client_state state;

    void listClients();

public:
    Ui(Communicator* comm);
    void readTerminal();
    void displayReceived();
};


#endif //NETWORKSYSTEMS_UI_H
