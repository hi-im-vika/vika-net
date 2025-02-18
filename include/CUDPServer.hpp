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
#include <queue>

class CUDPServer {
private:
    int _port = 0;                          ///< Port to listen on
    int _socket_fd = 0;                     ///< Socket file descriptor
    ssize_t _rx_code = 0;                   ///< Size of received data
    std::queue<std::string> _rx_time_queue; ///< Queue containing times of receive data
    struct sockaddr_in _server_addr{};      ///< Server info struct
    struct sockaddr_in _client_addr{};      ///< Client info struct
    socklen_t _client_addr_len = 0;         ///< Length of client address
    std::vector<uint8_t> _recv_buffer;      ///< Buffer for received data

    /**
     * @brief Internal function to init networking stuff
     * @return
     */
    bool init_net();

public:
    /**
     * @brief Constructor for CUDPServer
     */
    CUDPServer();

    /**
     * @brief Constructor for CUDPServer
     */
    ~CUDPServer();

    /**
     * @brief Set up UDP server on specified port
     * @param port  String with port to listen to
     */
    void setup(const std::string& port);

    /**
     * @brief Close and clean up UDP server
     */
    void setdn() const;

    /**
     * @brief           Receive data (nonblocking)
     * Meant to run in a loop in a thread.
     * @param rx_buf    Buffer to receive data into
     * @param src       struct containing info about data source
     * @param rx_bytes  Number of bytes received
     * @return          True if data was received, false otherwise
     */
    bool do_rx(std::vector<uint8_t> &rx_buf, sockaddr_in &src, long &rx_bytes);

    /**
     * @brief           Send data
     * Meant to run in a loop in a thread.
     * @param tx_buf    Buffer containing data to send
     * @param dst       struct containing destination
     * @return          True if data was sent, false otherwise
     */
    bool do_tx(const std::vector<uint8_t> &tx_buf, sockaddr_in &dst);
};
