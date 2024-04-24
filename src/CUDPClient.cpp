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
    // send prefix and ENQ
    std::string buffer = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + " \5";

    if (!(sendto(_socket_fd, buffer.data(), buffer.size(), 0,
                (struct sockaddr *) &_server_addr, sizeof(_server_addr)))) {
        spdlog::error("General error during ping tx");
        return false;
    }

    _server_addr_len = sizeof(_server_addr);
    _bytes_moved = 0;

    auto ping_timeout_start = std::chrono::steady_clock::now();
    buffer.clear();
    buffer.resize(UDP_MAX_SIZE);

    // spins until ping response or timeout
    while (_bytes_moved <= 0) {
        _bytes_moved = recvfrom(_socket_fd, buffer.data(), buffer.size(), 0,
                                (struct sockaddr *) &_server_addr, &_server_addr_len);
        if ((int) std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - ping_timeout_start).count() > PING_TIMEOUT) {
            spdlog::warn("No response to ping.");
            return false;
        }
    }

    // turn rx into ss
    std::stringstream rx_raw(std::string(buffer.begin(),buffer.begin() + _bytes_moved));

    // split ss into time and data
    std::string time, data;
    rx_raw >> time;
    rx_raw >> data;

    return (data == "\6");
}

bool CUDPClient::do_rx(std::vector<uint8_t> &rx_buf, long &rx_bytes) {

    // preallocate receiving buffer
    std::vector<uint8_t> rx_raw;
    rx_raw.clear();
    rx_raw.resize(UDP_MAX_SIZE);

    // get length of server address
    _server_addr_len = sizeof(_server_addr);

    // reset rx return code
    _rx_code = 0;

    // spins until complete response
    while (_rx_code <= 0 && _socket_ok) {
        // send data
        _rx_code = recvfrom(_socket_fd, rx_raw.data(), rx_raw.capacity(), 0,
                                (struct sockaddr *) &_server_addr, &_server_addr_len);
    }

    // turn rx into ss
    std::stringstream rx_raw_ss(std::string(rx_raw.begin(),rx_raw.begin() + _rx_code));

    // split ss into time and data
    std::string time, data;
    rx_raw_ss >> time;
    rx_raw_ss >> data;

    // measure response time
    _response_time_ms = (int) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - std::stol(time));
    if (data.empty()) {
        spdlog::error("Malformed data received");
        return false;
    }

    rx_buf = std::vector<uint8_t>(rx_raw.begin() + (int) time.length() + 1, rx_raw.end());
    rx_bytes = (long) (rx_raw.end() -  (rx_raw.begin() + (int) time.length() + 1));
    return true;
}

bool CUDPClient::do_tx(const std::vector<uint8_t> &tx_buf) {
    // check if socket is ok
    if (!_socket_ok) {
        spdlog::error("Socket error during tx");
        return false;
    }

    std::string now = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + " ";;
    std::vector<uint8_t> tx_this(now.begin(), now.end());
    tx_this.insert(tx_this.end(),tx_buf.begin(),tx_buf.end());

    // send message to server
    _tx_code = sendto(_socket_fd, tx_this.data(), tx_this.size(), 0,
                          (struct sockaddr *) &_server_addr, _server_addr_len);

    // if problem with sending data, return false
    if (_tx_code < 0) {
        spdlog::error("General error during tx");
        return false;
    }
    return true;
}

bool CUDPClient::get_socket_status() {
    return _socket_ok;
}

int CUDPClient::get_last_response_time() {
    return _response_time_ms;
}