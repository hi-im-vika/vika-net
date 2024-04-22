/**
 * CUDPClient.cpp - UDP client code
 * 2024-04-15
 * vika <https://github.com/hi-im-vika>
 */

// try debugging by removing all image and only keep commands
// try decreasing timeout or even having a timeout at all
// try having multiple ports, one for handling image data, one for handling commands
// packet id to keep track of data
// try queue design (refer to template)
// emergency queue, flush regular queue

#include "../include/CUDPClient.hpp"

CUDPClient::CUDPClient() = default;

CUDPClient::~CUDPClient() {
    setdn();
}

bool CUDPClient::init_net() {
    spdlog::info("Beginning socket init.");
    if (_host.empty() || !_port) {
        spdlog::error("No host/port specified.");
        return false;
    }

    // create new socket, exit on failure
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        spdlog::error("Error opening socket");
        return false;
    }

    spdlog::info("Setting socket to nonblocking.");
    int flags = fcntl(_socket_fd, F_GETFL, 0);
    fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);

    spdlog::info("Connecting to " + _host + ":" + std::to_string(_port));

    // set server details
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(_port);
    _server_addr.sin_addr.s_addr = inet_addr(_host.data());

    do {
        spdlog::info("Sending ENQ...");
        _socket_ok = ping();
        if (_socket_ok) {
            spdlog::info("ACK received");
        } else {
            spdlog::info("Timeout");
        }
    } while (!_socket_ok);

    spdlog::info("Sending to udp://" + _host + ":" + std::to_string(_port));
    return true;
}

void CUDPClient::setup(const std::string &host, const std::string &port) {
    spdlog::info("Beginning UDP client setup.");
    _host = host;
    _port = std::stoi(port);
    if (!init_net()) {
        spdlog::error("Error during UDP client setup. Shutting down!");
        exit(1);
    }
}

void CUDPClient::setdn() const {
    close(_socket_fd);
}

bool CUDPClient::ping() {
    // assemble ping packet with counter
    std::string ping_string = "C" + std::to_string(_packet_counter) + "|" + "\5";
    std::vector<uint8_t> ping_buffer(ping_string.begin(), ping_string.end());

    if (!(sendto(_socket_fd, ping_buffer.data(), ping_buffer.capacity(), 0,
                 (struct sockaddr *) &_server_addr, sizeof(_server_addr)))) {
        spdlog::error("General error during ping tx");
        return false;
    }

    _server_addr_len = sizeof(_server_addr);
    _bytes_moved = 0;
    ping_buffer.clear();
    ping_buffer.resize(UDP_MAX_SIZE);

    auto ping_timeout_start = std::chrono::steady_clock::now();

    // spins until ping response or timeout
    while (_bytes_moved <= 0) {
        _bytes_moved = recvfrom(_socket_fd, ping_buffer.data(), ping_buffer.capacity(), 0,
                                (struct sockaddr *) &_server_addr, &_server_addr_len);
        if ((int) std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - ping_timeout_start).count() > PING_TIMEOUT) {
            spdlog::warn("No response to ping.");
            return false;
        }
    }
    auto data_begin = std::find(ping_buffer.begin(), ping_buffer.end(), '|');
    if (data_begin++ == ping_buffer.end()) {
        spdlog::error("Separator not found");
        exit(0);
    }
    ping_buffer = std::vector<uint8_t>(data_begin,ping_buffer.begin() + _bytes_moved);
    spdlog::info(std::string(ping_buffer.begin(), ping_buffer.end()));
    return (ping_buffer.at(0) == '\6');
}

bool CUDPClient::do_rx(std::vector<uint8_t> &rx_buf, long &rx_bytes) {

    // preallocate receiving buffer
    rx_buf.clear();
    rx_buf.resize(UDP_MAX_SIZE);

    // get length of server address
    _server_addr_len = sizeof(_server_addr);

    // reset rx return code
    _bytes_moved = 0;

    // spins until complete response
    while (_bytes_moved <= 0 && _socket_ok) {
        // send data
        _bytes_moved = recvfrom(_socket_fd, rx_buf.data(), rx_buf.capacity(), 0,
                                (struct sockaddr *) &_server_addr, &_server_addr_len);
    }

    rx_bytes = _bytes_moved;
    return true;
}

bool CUDPClient::do_tx(const std::vector<uint8_t> &tx_buf) {
    // check if socket is ok
    if (!_socket_ok) {
        spdlog::error("Socket error during tx");
        return false;
    }

    // add packet counter

    // reset packet counter if too high
    _packet_counter = _packet_counter > 65535 ? 0 : _packet_counter;

    // assemble final packet with counter and data
    std::string packet_identifier = "C" + std::to_string(_packet_counter++) + "|";
    std::vector<uint8_t> tx_this(packet_identifier.begin(),packet_identifier.end());
    tx_this.insert(tx_this.end(),tx_buf.begin(),tx_buf.end());
    spdlog::info(std::string(tx_this.begin(), tx_this.end()));

    // send message to server
    _bytes_moved = sendto(_socket_fd, tx_this.data(), tx_this.size(), 0,
                          (struct sockaddr *) &_server_addr, _server_addr_len);

    // if problem with sending data, return false
    if (_bytes_moved < 0) {
        spdlog::error("General error during tx");
        return false;
    }

    return true;
}

bool CUDPClient::get_socket_status() {
    return _socket_ok;
}
