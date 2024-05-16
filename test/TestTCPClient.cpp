/**
 * TestTCPClient.cpp - new file
 * 2024-05-11
 * vika <https://github.com/hi-im-vika>
 */

#include <iostream>
#include <queue>
#include <csignal>

#include "../include/CTCPClient.hpp"

#define PING_TIMEOUT 1000
#define NET_DELAY 35
#define TCP_DELAY 30

volatile sig_atomic_t stop;
volatile bool send_data = true;
volatile bool stop_main = false;

void catch_signal(int sig) {
//    stop = 1;
    stop_main = true;
}

void do_listen(CTCPClient *c, std::queue<std::string> *q) {
    std::vector<uint8_t> rx_buf;
    long rx_bytes;
    while (stop != 1) {
        rx_bytes = 0;
        rx_buf.clear();
//        spdlog::info("Listening");
        c->do_rx(rx_buf,rx_bytes);
        std::string temp = std::string(rx_buf.begin(),rx_buf.begin() + rx_bytes);
        // only add to rx queue if data is not empty
        if(!temp.empty()) q->emplace(temp);
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(TCP_DELAY));
    }
}

void do_send(CTCPClient *c, std::queue<std::string> *q) {
    while (stop != 1) {
        for (; !q->empty(); q->pop()) {
//            spdlog::info("Sending");
            std::vector<uint8_t> tx_buf(q->front().begin(),q->front().end());
            c->do_tx(tx_buf);
            std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(TCP_DELAY));
        }
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(NET_DELAY));
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>" << std::endl;
        return 1;
    }

    std::queue<std::string> tx_queue, rx_queue;
    std::chrono::steady_clock::time_point timeout_count;

    signal(SIGINT, catch_signal);
    CTCPClient c = CTCPClient();
    c.setup(argv[1], argv[2]);

    timeout_count = std::chrono::steady_clock::now();
    send_data = c.get_socket_status();
    // start listen thread
    std::thread thread_for_listening(do_listen, &c, &rx_queue);
    thread_for_listening.detach();

    // start send thread
    std::thread thread_for_sending(do_send, &c, &tx_queue);
    thread_for_sending.detach();
    while(!stop_main) {
        for (; !rx_queue.empty(); rx_queue.pop()) {

//            // acknowledge next data in queue
            spdlog::info("New in RX queue with size: " + std::to_string(rx_queue.front().size()));
            spdlog::info("Content: " + std::string(rx_queue.front().begin(), rx_queue.front().end()));
            spdlog::info("Remaining in queue: " + std::to_string(rx_queue.size()));

            // reset timeout
            // placement of this may be a source of future bug
            timeout_count = std::chrono::steady_clock::now();
        }
        // Send data
        tx_queue.emplace("G 1");
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1));
    }

    // tx EOT to stop
    spdlog::info("Stopping nicely");

    // wait for send stop...
    std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
    stop = 1;
    if (thread_for_listening.joinable()) thread_for_listening.join();
    if (thread_for_sending.joinable()) thread_for_sending.join();
    spdlog::info("Goodbye");
    return 0;
}