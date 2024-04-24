/**
 * CUDPClient.h - UDP client header
 * 2024-04-15
 * vika <https://github.com/hi-im-vika>
 */

#pragma once

#define PING_TIMEOUT 1000
#define UDP_MAX_SIZE 65535

#include <thread>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <queue>

class CUDPClient {
private:
    bool init_net();

    std::string _host;
    int _port = 0;
    int _socket_fd = 0;
    bool _socket_ok = false;
    ssize_t _bytes_moved = 0;
    ssize_t _rx_code = 0;
    ssize_t _tx_code = 0;
    struct sockaddr_in _server_addr{};
    socklen_t _server_addr_len = 0;
    int _response_time_ms = 0;

public:
    CUDPClient();
    ~CUDPClient();
    void setup(const std::string& host, const std::string& port);
    void setdn() const;
    bool do_rx(std::vector<uint8_t> &rx_buf, long &rx_bytes);
    bool do_tx(const std::vector<uint8_t> &tx_buf);
    bool ping();

    bool get_socket_status();
    int get_last_response_time();
};
