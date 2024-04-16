/**
 * CUDPServer.h - UDP server header
 * 2024-04-15
 * vika <https://github.com/hi-im-vika>
 */

#pragma once

#define UDP_MAX_SIZE 65535

#include <thread>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <spdlog/spdlog.h>

class CUDPServer {
private:
    bool init_net();

    int _port = 0;
    int _socket_fd = 0;
    ssize_t _read_code = 0;
    struct sockaddr_in _server_addr{};
    struct sockaddr_in _client_addr{};
    socklen_t _client_addr_len = 0;
    std::vector<uint8_t> _recv_buffer;
public:
    CUDPServer();
    ~CUDPServer();
    void setup(const std::string& port);
    void setdn() const;
    bool do_rx(std::vector<uint8_t> &rx_buf, sockaddr_in &src, long &rx_bytes);
    bool do_tx(const std::vector<uint8_t> &tx_buf, sockaddr_in &dst);
};
