# vika-net
This is a simple C++ networking library created for my embedded projects. It is designed so that a client can send data to a server and receive a response. It also includes a timestamp in the data sent so latency can be measured.

Currently, it only supports UDP, but I am working on TCP support.
## Intro
While using the UDP library from Boost, I found that there was an unexplained delay when transmitting UDP packets. I also didn't have the time to learn how to use other libraries that I felt weren't completely suited for my purpose, so I wrote my own.
### How does it work?
Client
1. Data to be sent is added to a queue, which is serviced asynchronously by a thread.
2. The most recent item in the queue is looked at.
3. A timestamp is prefixed to the data.
4. The data is sent to the server.

Server
1. Data is received from the client.
2. The data is separated into the timestamp and the actual data.
3. The timestamp and data are added to separate queues.
4. Incoming data is processed and a response is added to the sending queue.
5. The timestamp from earlier is prefixed to the data to be sent.
6. The data is sent to the client.

Client
1. Steps 1 to 3 on the server are performed on the client.
2. The incoming data is processed as needed.
3. The incoming timestamp is compared with the current time and a latency measurement is calculated from their difference.

## Usage
Add the following to `vendor/CMakeLists.txt`:
```cmake
# vendor/CMakeLists.txt

include(FetchContent)

FetchContent_Declare(
        vika-net
        GIT_REPOSITORY "https://github.com/hi-im-vika/vika-net.git"
        GIT_TAG         origin/main
)

add_subdirectory(vika-net)
```
Add the library to your root `CMakeLists.txt`:
```cmake
target_link_libraries(<project_name> vika-net <other_libraries>)
```

This library should build with your project now.