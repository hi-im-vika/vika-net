/**
 * CUDPServer.cpp - UDP server code
 * 2024-04-15
 * vika <https://github.com/hi-im-vika>
 */

#include "../include/CUDPServer.hpp"

CUDPServer::CUDPServer() = default;

CUDPServer::~CUDPServer() {
    setdn();
}

bool CUDPServer::init_net() {

    spdlog::info("Beginning socket init.");
    if (!_port) {
        spdlog::error("No port specified.");
        return false;
    }

    // create new socket, exit on failure
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        spdlog::error("Error opening socket");
        return false;
    }

    // bind program to address and port
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(_port);
    _server_addr.sin_addr.s_addr = INADDR_ANY;

    if ((bind(_socket_fd, (struct sockaddr *) &_server_addr, sizeof(_server_addr))) < 0) {
        spdlog::error("Error opening socket");
        close(_socket_fd);
        return false;
    }

    spdlog::info("Listening on udp://0.0.0.0:" + std::to_string(_port));
    spdlog::info("Socket init complete.");
    return true;
}

bool CUDPServer::do_rx(
        std::vector<uint8_t> &rx_processed,
        sockaddr_in &src,
        long &rx_bytes) {
    std::vector<uint8_t> rx_buf;
    rx_buf.clear();
    rx_buf.resize(UDP_MAX_SIZE);
    _rx_code = 0;

    // listen for client
    _client_addr_len = sizeof(_client_addr);
    _rx_code = recvfrom(_socket_fd, rx_buf.data(), rx_buf.capacity(), 0, (struct sockaddr *) &_client_addr, &_client_addr_len);
    if (_rx_code < 0) {
        spdlog::error("Error reading data.");
        close(_socket_fd);
        return false;
    }
    auto data_begin = std::find(rx_buf.begin(),rx_buf.end(),'|');
    if (data_begin++ == rx_buf.end()) {
        spdlog::error("Separator not found");
        exit(0);
    }

    _packet_counter = std::stoi(std::string(rx_buf.begin() + 1,data_begin - 1));
    std::vector<uint8_t> actual_data(data_begin,rx_buf.begin() + _rx_code);
    if(actual_data.at(0) == '\5') {
        spdlog::info("sending ping");
        std::string header_ctr("C" + std::to_string(_packet_counter) + "|" + "\6");
        do_tx(std::vector<uint8_t>(header_ctr.begin(), header_ctr.end()), _client_addr);
    }
    rx_processed = actual_data;
    rx_bytes = actual_data.size();
    src = _client_addr;
    return true;
}

bool CUDPServer::do_tx(const std::vector<uint8_t> &tx_buf,
                       sockaddr_in &dst) {
    // respond to client
    if (sendto(_socket_fd, tx_buf.data(), tx_buf.size(), 0, (struct sockaddr *) &dst, sizeof(dst)) <
        0) {
        spdlog::error("Error sending data.");
        close(_socket_fd);
        return false;
    }
    return true;
}

void CUDPServer::setup(const std::string &port) {
    spdlog::info("Beginning UDP server setup.");
    _port = std::stoi(port);
    if (!init_net()) {
        spdlog::error("Error during UDP server setup. Shutting down!");
        exit(1);
    }
}

void CUDPServer::setdn() const {
    close(_socket_fd);
}
