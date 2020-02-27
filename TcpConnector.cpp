//
// Created by jaimie on 7-10-19.
//

#include "TcpConnector.h"

#include <mutex>
#include <limits>
#include <cstring>

#include "connectivity.h"
#include "transportPacket.h"

#include <fstream>
#include <iostream>
#include <random>

std::mutex in_flight_mutex;

TcpConnector::TcpConnector(Communicator* comm, ip_t sender, ip_t receiver) : comm(comm), sender(sender), receiver(receiver) {
    comm->logFile << "Creating new TCP connector" << std::endl;
    connection_status = CONNECTION_STATUS::LISTEN;
    fileData.open("testClient.txt");
    distribution = std::uniform_int_distribution<uint32_t>(0, UINT32_MAX);
}

TcpConnector::~TcpConnector() {
    fileData.close();
}

packet_header TcpConnector::getIPheader() {
    packet_header h;
    h.source_address = sender;
    h.proxy_address = comm->getConnectivity()->getNextHop(receiver);
    h.destination_address = receiver;

    h.payload_size = sizeof(transport_header);
    h.payload_type = TRANSPORT_DATA;
    h.sender_nonce = comm->getNonce();
    return h;
}

transport_header TcpConnector::getTransportHeader(uint32_t sequence, uint32_t acknowledgement) {
    transport_header transportHeader;

    /*
    transportHeader.source_port = source_port;
    transportHeader.destination_port = destination_port;
    */
    transportHeader.source_port = 1234;
    transportHeader.destination_port = 1234;

    transportHeader.sequence_number = sequence;
    transportHeader.acknowledgement_number = acknowledgement;

    //TODO: proper values
    transportHeader.flags = 0xFF;
    transportHeader.windowSize = 2;

    return transportHeader;
}

uint32_t TcpConnector::randomSequenceNumber() {
    return distribution(generator);
    /*
    if (connection_status == LISTEN) {
        return 100;
    } else {
        return 200;
    }
    */
}

uint32_t TcpConnector::getHighestNumberOverlow(uint32_t a, uint32_t b) {
    uint32_t halfway = (UINT32_MAX >> 1);
    uint32_t lowerQuarter = (UINT32_MAX >> 2);
    uint32_t upperQuarter = halfway + lowerQuarter;
    if ((a < lowerQuarter) && ( b > upperQuarter)) {
        return a;
    } else if ((a > upperQuarter) && (b < lowerQuarter)) {
        return b;
    } else {
        return std::max(a,b);
    }
}


void TcpConnector::sendSynPacket() {
    transport_header transportHeader = getTransportHeader(randomSequenceNumber(),0);
    transportHeader.flags = FLAGS_SYN;

    seq_number_last_send = transportHeader.sequence_number;

    sendPacketImmediately(std::make_unique<TransportPacket>(comm, getIPheader(), &transportHeader, (char*) nullptr));
}

void TcpConnector::sendSynAckPacket(uint32_t acknowledgement){
    transport_header transportHeader = getTransportHeader(randomSequenceNumber(), acknowledgement);
    transportHeader.flags = FLAGS_SYN | FLAGS_ACK;

    seq_number_last_send = transportHeader.sequence_number;

    sendPacketImmediately(std::make_unique<TransportPacket>(comm, getIPheader(), &transportHeader, (char*) nullptr));
}

void TcpConnector::sendAckPacket(uint32_t sequence, uint32_t acknowledgement) {
    transport_header transportHeader = getTransportHeader(sequence, acknowledgement);
    transportHeader.flags = FLAGS_ACK;
    seq_number_last_send = transportHeader.sequence_number;

    sendPacketImmediately(std::make_unique<TransportPacket>(comm, getIPheader(), &transportHeader, (char*) nullptr));
}


void TcpConnector::sendPacketImmediately(std::unique_ptr<TransportPacket> pkt) {
    comm->sendPacket(pkt.get());
    data_packet_node node;
    node.data_packet = std::move(pkt);

    node.timeSent = std::chrono::system_clock::now();
    in_flight_list.push_back(std::move(node));
}

void TcpConnector::sendQueuedPackets() {
    for (const auto& packet_it : packets_to_be_send) {
        transport_header transport_header;
        std::unique_ptr<TransportPacket> packet_blarg;
        if (connection_status == ESTABLISHED) {
            comm->logFile << "Packet header claimed size (1): " << packet_it->getPacketHeader().payload_size << std::endl;
            transport_header = getTransportHeader(seq_number_last_send + 1,
                                                                   ack_number_last_received);
            transport_header.flags = FLAGS_ACK;
            seq_number_last_send++;
            packet_blarg = std::move(std::make_unique<TransportPacket> (comm, packet_it->getPacketHeader(),
                                                                &transport_header, packet_it->getData() +
                                                                                   sizeof(packet_header) +
                                                                                   sizeof(transport_header)));



        } else {
            transport_header = packet_it->getTransportHeader();
            comm->logFile << "Packet header claimed size (2): " << packet_it->getPacketHeader().payload_size << std::endl;
            packet_blarg = std::move(std::make_unique<TransportPacket> (comm, packet_it->getPacketHeader(),
                                               &transport_header, packet_it->getData() +
                                                                  sizeof(packet_header) +
                                                                  sizeof(transport_header)));
        }

        comm->sendPacket(packet_blarg.get());

        data_packet_node node;
        node.data_packet = std::move(packet_blarg);


        node.timeSent = std::chrono::system_clock::now();
        in_flight_list.push_back(std::move(node));
    }
    packets_to_be_send.clear();
}

void TcpConnector::reorderPackets() {
    bool packetPushed;
    do {
        packetPushed = false;
        //TODO: reorder_buffer is now never cleared
        //for (auto& it : reorder_buffer) {
        for(std::list<std::unique_ptr<TransportPacket>>::iterator it = reorder_buffer.begin(); it != reorder_buffer.end();) {
            ack_number_last_received = getHighestNumberOverlow(ack_number_last_received, (*it)->getTransportHeader().acknowledgement_number);


            if ((*it)->getTransportHeader().sequence_number == seq_number_last_received + 1) {
                seq_number_last_received++;
                packetPushed = true;

                char *data = nullptr;
                int length;
                (*it)->getContent(&data, &length);
                if (length != 0) {
                    comm->logFile << "Received packet for application with payload (1)" << std::endl;
                    std::cout << receiver << ": ";
                    for(int i=0 ; i<length; i++) {
                        std::cout << data[i];
                    }
                    std::cout << std::endl;
                    fileData.write(data, length);
                    fileData.flush();
                } else {
                    comm->logFile << "No data was attached" << std::endl;
                }
                it = reorder_buffer.erase(it);

            //Received duplicate packets, since current ack number is higher or equal to that of the packet
            } else if (seq_number_last_received == getHighestNumberOverlow((*it)->getTransportHeader().sequence_number, seq_number_last_received)) {
                comm->logFile << "Received duplicate packets, deleting them" << std::endl;
                comm->logFile << "Packet sequence number is " << (*it)->getTransportHeader().sequence_number << " last received is " << seq_number_last_received << std::endl;
                char *data = nullptr;
                int length;
                (*it)->getContent(&data, &length);
                if (length != 0) {
                    comm->logFile << "Received packet for application with payload (2)" << std::endl;
                } else {
                    comm->logFile << "No data was attached" << std::endl;
                }
                it = reorder_buffer.erase(it);
            //Missing packets, waiting for missing packets
            } else {
                it++;
            }
        }
    } while (packetPushed);


    //ack_number_last_received is updated, all numbers which are lower are hence received

    for (std::list<data_packet_node>::iterator it = in_flight_list.begin(); it != in_flight_list.end(); ) {
        uint32_t seq_num = it->data_packet->getTransportHeader().sequence_number;
        if (ack_number_last_received == getHighestNumberOverlow(ack_number_last_received, seq_num)) {
            it = in_flight_list.erase(it);
            comm->logFile << "Erased packet in flight" << std::endl;
        } else {
            it++;
        }
    }
}

/**
 * SendData will send data across a "TCP" socket
 * @param data to be send
 * @param length, length of the data to be send
 * @return
 */

bool TcpConnector::sendData(char* data, int length) {
    in_flight_mutex.lock();
    packet_header p = getIPheader();
    transport_header h = getTransportHeader(seq_number_last_send + 1, ack_number_last_received);
    p.payload_size += length;
    comm->logFile << "Calling from sendData function" << std::endl;
    bool status = sendPacket( std::make_unique<TransportPacket>(comm, p, &h, data));
    in_flight_mutex.unlock();
    return status;
}


bool TcpConnector::sendPacket(std::unique_ptr<TransportPacket> ptr) {
    switch (connection_status) {
        case LISTEN:
            comm->logFile << "Sending sync packet" << std::endl;
            sendSynPacket();
            connection_status = SYN_SENT;
            break;
        case SYN_SENT:
        case SYN_RECEIVED:
        case ESTABLISHED:

            break;

        //Active close
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSING:
        case TIME_WAIT:

        //Passive close
        case CLOSE_WAIT:
        case LAST_ACK:
            return false;
    }

    packets_to_be_send.push_back(std::move(ptr));

    if (connection_status == ESTABLISHED) {
        sendQueuedPackets();
    }

    return true;
}

/**
 * ReceivePacket, call when a packet is received and the TCP socket should process it
 * @param p, the packet the TCP connection should process
 */

void TcpConnector::receivePacket(std::unique_ptr<TransportPacket> p) {
    in_flight_mutex.lock();

    comm->logFile << "Handeling transport packet" << std::endl;

    if (connection_status == LISTEN) {
        if (p->getTransportHeader().flags & FLAGS_SYN) {
            comm->logFile << "SYN packet detected" << std::endl;

            seq_number_last_received = p->getTransportHeader().sequence_number;

            sendSynAckPacket(seq_number_last_received);
            connection_status = SYN_RECEIVED;
        }
    } else if (connection_status == SYN_SENT) {
        comm->logFile << "State is SYN sent, received packed" << std::endl;
        if ((p->getTransportHeader().flags & FLAGS_ACK) && (p->getTransportHeader().flags & FLAGS_SYN)) {
            comm->logFile << "ACK en SYN are set" << std::endl;

            seq_number_last_received = p->getTransportHeader().sequence_number;
            comm->logFile << "Sequence number now is:" << seq_number_last_received << std::endl;


            if (seq_number_last_send == p->getTransportHeader().acknowledgement_number) {
                comm->logFile << "Acknowledgement number is correct" << std::endl;
                ack_number_last_received = p->getTransportHeader().acknowledgement_number;

                sendAckPacket(seq_number_last_send+1, p->getTransportHeader().sequence_number);
                connection_status = ESTABLISHED;

                //Die alle packeten pushed. Volgens mij worden de packeten verwijdert voordat ze goed gestuurd kunnen worden.
                comm->logFile << "Connection is ESTABLISHED :)" << std::endl;

                reorder_buffer.push_back(std::move(p));
                reorderPackets();

                sendQueuedPackets();
            } else {
                comm->logFile << "This is not the acknowledgement number I was expecting" << std::endl;
            }
        } else if (p->getTransportHeader().flags & FLAGS_SYN) {
            comm->logFile << "Only SYN is set, weird" << std::endl;
            seq_number_last_received = p->getTransportHeader().sequence_number;

            sendAckPacket(seq_number_last_send+1, p->getTransportHeader().sequence_number);
            connection_status = SYN_RECEIVED;
        } else {
            comm->logFile << "WAIT WHUT?" << std::endl;
        }
    } else if (connection_status == SYN_RECEIVED) {
        if (p->getTransportHeader().flags & FLAGS_ACK) {
            //seq_number_last_received = p->getTransportHeader()->sequence_number;
            ack_number_last_received = p->getTransportHeader().acknowledgement_number;
            comm->logFile << "ACK packet detected" << std::endl;
            connection_status = ESTABLISHED;
            comm->logFile << "Connection is ESTABLISHED :)" << std::endl;

            reorder_buffer.push_back(std::move(p));
            reorderPackets();

            sendQueuedPackets();
        }
    } else if (connection_status == ESTABLISHED) {
        reorder_buffer.push_back(std::move(p));
        reorderPackets();
    }
    in_flight_mutex.unlock();
}

void TcpConnector::checkRetransmission() {
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();

    comm->logFile << "Amount of packets to be retransmitted: " << in_flight_list.size() << std::endl;

    for (auto& packet : in_flight_list) {
        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds >(
                currentTime - packet.timeSent
        );

        if (duration.count() > TCP_RETRANSMIT_TIMEOUT_MS) {
            uint32_t payloadSize = packet.data_packet->getPacketHeader().payload_size;

            //Get IP header will update the proxy IP
            packet_header packet_header = getIPheader();
            packet_header.payload_size = payloadSize;
            packet.data_packet->updateIPheader(packet_header);
            comm->sendPacket(packet.data_packet.get());
            packet.timeSent = currentTime;
        }
    }
}

void TcpConnector::update() {
    in_flight_mutex.lock();
    checkRetransmission();
    in_flight_mutex.unlock();
}

void TcpConnector::closeConnection() {
    switch (connection_status) {
        case LISTEN:
            //HA nope
        case SYN_SENT:
            connection_status = LISTEN;
            break;
        case SYN_RECEIVED:
            //TODO: send FIN
            connection_status = FIN_WAIT_1;
            break;
        case ESTABLISHED:
            //TODO: set some timers or something
            connection_status = FIN_WAIT_1;
            break;

        //TODO: implement these case
        //Active close
        case FIN_WAIT_1:
        case FIN_WAIT_2:
        case CLOSING:
        case TIME_WAIT:
            break;

        //Passive close
        case CLOSE_WAIT:
            //TODO: send FIN packet
            connection_status = LAST_ACK;
            break;
        case LAST_ACK:
            break;
    }
}