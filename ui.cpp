//
// Created by jaimie on 2018-10-30.
//

#include <string>
#include <iostream>

#include "ui.h"


//TODO: okay I think this class was supposed to be the keyboard interface controller.
//Right now however it is not really used anywhere.
Ui::Ui(Communicator* comm): comm(comm) {

}

void Ui::listClients() {

}

void Ui::readTerminal() {
    std::string input;
    getline(std::cin, input);
    comm->getTransmitBuffer()->push(input);
}

void Ui::displayReceived() {
    std::string output = comm->getReceiveBuffer()->pop();
    std::cout << output << std::endl;
}