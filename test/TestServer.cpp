/**
 * TestServer.cpp - Server testing code
 * 2024-04-15
 * vika <https://github.com/hi-im-vika>
 */

#include <iostream>
#include <queue>
#include <csignal>

#include "../include/CUDPServer.hpp"

#define PING_TIMEOUT 1000
#define NET_DELAY 1

volatile sig_atomic_t stop;
volatile bool send_data = false;

void catch_signal(int sig) {
    stop = 1;
}

void do_listen(CUDPServer *s, std::queue<std::string> *q, sockaddr_in *src) {
    std::vector<uint8_t> rx_buf;
    long rx_bytes;
    while (stop != 1) {
        rx_bytes = 0;
        rx_buf.clear();
        s->do_rx(rx_buf, *src, rx_bytes);
        std::string temp = std::string(rx_buf.begin(),rx_buf.begin() + rx_bytes);
        // only add to rx queue if data is not empty
        if(!temp.empty()) q->emplace(temp);
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(NET_DELAY));
    }
}

void do_send(CUDPServer *s, std::queue<std::string> *q, sockaddr_in *dst) {
    int number = 0;
    while (stop != 1) {
        if (send_data) q->emplace("sample text");
        for (; !q->empty(); q->pop()) {
            spdlog::info("Sending");
            std::vector<uint8_t> tx_buf(q->front().begin(),q->front().end());
            s->do_tx(tx_buf,*dst);
        }
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(NET_DELAY));
    }
}

int main() {
    sockaddr_in client{};
    std::queue<std::string> tx_queue, rx_queue;
    std::chrono::steady_clock::time_point timeout_count;
    int time_since_start;

    signal(SIGINT, catch_signal);
    CUDPServer c = CUDPServer();
    c.setup("46188");

    // start listen thread
    std::thread thread_for_listening(do_listen, &c, &rx_queue, &client);
    thread_for_listening.detach();

    // start send thread
    std::thread thread_for_sending(do_send, &c, &tx_queue, &client);
    thread_for_sending.detach();

    while(!stop) {
        /*
         * This models the update loop in a real program.
         * Process the rx queue, FIFO
         */
        for (; !rx_queue.empty(); rx_queue.pop()) {

            // acknowledge next data in queue
            spdlog::info("New in RX queue: " + rx_queue.front());
            spdlog::info("Remaining in queue: " + std::to_string(rx_queue.size()));

            // respond to ping
            if(rx_queue.front()[0] == '\5') {
                tx_queue.emplace("\6");
                // it's just a ping packet, move on
                continue;
            }

            std::string action = rx_queue.front().substr(0,3);

            // respond to i'm alive command
            if(action == "A 0") {
                timeout_count = std::chrono::steady_clock::now();
                continue;
            }

            // respond to begin get data command
            if(action == "S 1") {
//                send_data = false;
//                std::stringstream ss;
//                std::string temp;
//                ss.str(rx_queue.front());
//                while(ss >> temp) {
//                    spdlog::info(temp);
//                }
                spdlog::info(rx_queue.front());
                continue;
            }

            // respond to begin get data command
            if(action == "S 0") {
                send_data = false;
                continue;
            }

            // respond to begin get data command
            if(action == "G 0") {
                send_data = true;
                timeout_count = std::chrono::steady_clock::now();
                continue;
            }
        }

        time_since_start = (int) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeout_count).count();
        if (send_data) {
//            spdlog::info("Time since last client data: " + std::to_string(time_since_start));
            if (time_since_start > PING_TIMEOUT) {
                spdlog::warn("Client is gone...");
                send_data = false;
            }
        }
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(NET_DELAY));
    }

    spdlog::info("Stopping nicely");
    if (thread_for_listening.joinable()) thread_for_listening.join();
    if (thread_for_sending.joinable()) thread_for_sending.join();
    spdlog::info("Goodbye");
    return 0;
}