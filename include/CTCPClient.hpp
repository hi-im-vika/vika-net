/**
 * CTCPClient.hpp - new file
 * 2024-05-11
 * vika <https://github.com/hi-im-vika>
 */


#pragma once

#pragma once

#define TCP_TIMEOUT 100
#define UDP_MAX_SIZE 65535

#include <thread>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

#ifdef WIN32
#include "Winsock2.h"
#include <ws2tcpip.h>
#else
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <spdlog/spdlog.h>

class CTCPClient {
private:
    bool init_net();

#ifdef WIN32
    WSADATA _wsdat;                         ///< Winsock object
#endif
    std::string _host;
    int _port = 0;
    int _socket_fd = 0;
    bool _socket_ok = false;
    ssize_t _rx_code = 0;
    ssize_t _tx_code = 0;
    struct sockaddr_in _server_addr{};
    socklen_t _server_addr_len = 0;

public:
    CTCPClient();
    ~CTCPClient();
    void setup(const std::string& host, const std::string& port);
    void setdn() const;
    bool do_rx(std::vector<uint8_t> &rx_buf, long &rx_bytes);
    bool do_tx(const std::vector<uint8_t> &tx_buf);

    bool get_socket_status();
};
