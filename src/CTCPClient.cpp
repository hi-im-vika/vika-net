/**
 * CTCPClient.cpp - new file
 * 2024-05-11
 * vika <https://github.com/hi-im-vika>
 */

#include "../include/CTCPClient.hpp"

CTCPClient::CTCPClient() = default;

CTCPClient::~CTCPClient() {
    setdn();
}

bool CTCPClient::init_net() {
    spdlog::info("Beginning socket init.");
    if (_host.empty() || !_port) {
        spdlog::error("No host/port specified.");
        return false;
    }

#ifdef WIN32
    // initialize winsock
    if (WSAStartup(0x0101, &_wsdat)) {
        WSACleanup();
        return false;
    }
#endif

    // create new socket, exit on failure
    if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        spdlog::error("Error opening socket");
#ifdef WIN32
        WSACleanup();
#endif
        return false;
    }

    spdlog::info("Setting socket to nonblocking.");
    int flags = fcntl(_socket_fd, F_GETFL, 0);

    // set nonblocking
    if (fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        spdlog::error("Error setting socket to nonblocking");
        return false;
    }

    spdlog::info("Connecting to " + _host + ":" + std::to_string(_port));

    // set server details
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(_port);
    _server_addr.sin_addr.s_addr = inet_addr(_host.data());
    _server_addr_len = sizeof(_server_addr);

    // expect -1 here, but why??
    connect(_socket_fd, (struct sockaddr*)&_server_addr, _server_addr_len);

    spdlog::info("Sending to tcp://" + _host + ":" + std::to_string(_port));

    _socket_ok = true;

    return true;
}

void CTCPClient::setup(const std::string &host, const std::string &port) {
    spdlog::info("Beginning TCP client setup.");
    _host = host;
    _port = std::stoi(port);
    if (!init_net()) {
        spdlog::error("Error during TCP client setup. Shutting down!");
        exit(1);
    }

    // need to add small wait or else setup fails (socket doesn't get set up fast enough?)
    std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(20));
}

void CTCPClient::setdn() const {
    close(_socket_fd);
}

bool CTCPClient::do_rx(std::vector<uint8_t> &rx_buf, long &rx_bytes) {
    // check if socket is ok
    if (!_socket_ok) {
        spdlog::error("Socket error during tx");
        return false;
    }

    // preallocate receiving buffer
    std::vector<uint8_t> rx_raw;
    rx_raw.clear();
    rx_raw.resize(UDP_MAX_SIZE);

    // reset rx return code
    _rx_code = 0;

    auto timeout_start = std::chrono::steady_clock::now();
    auto time_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - timeout_start).count();

    // spins until complete response
    while (_rx_code <= 0 && _socket_ok && (int) time_since_start < TCP_TIMEOUT) {
        // rx data
        time_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - timeout_start).count();
        _rx_code = recv(_socket_fd, rx_raw.data(), rx_raw.capacity(), 0);
    }

    if (time_since_start >= TCP_TIMEOUT) {
        spdlog::warn("TCP Timed out");
        return false;
    }

    if (_rx_code < 0) {
        spdlog::error("General error during rx");
        return false;
    }

    if (!_rx_code) {
        spdlog::warn("No data received");
        return false;
    }

    rx_buf = std::vector<uint8_t>(rx_raw.begin(), rx_raw.begin() + _rx_code);
    rx_bytes = (long) rx_buf.size();
    return true;
}

bool CTCPClient::do_tx(const std::vector<uint8_t> &tx_buf) {
    // check if socket is ok
    if (!_socket_ok) {
        spdlog::error("Socket error during tx");
        return false;
    }

    // do tcp send
    _tx_code = send(_socket_fd, tx_buf.data(), tx_buf.size(), 0);

    // if problem with sending data, return false
    if (_tx_code < 0) {
        spdlog::error("General error during tx");
        return false;
    }
    return true;
}

bool CTCPClient::get_socket_status() {
    return _socket_ok;
}