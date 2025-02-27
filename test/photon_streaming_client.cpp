#include "co_context/net.hpp"
#include <string_view>
using namespace co_context;
using Socket = co_context::socket;

constexpr std::string_view hostname{"127.0.0.1"};
constexpr uint16_t port = 9527;
constexpr size_t buf_size = 512;

task<> recv_session(Socket sock) {
    char buf[buf_size];

    while (true) {
        int nr = co_await sock.recv(buf);
        if (nr <= 0) {
            break;
        }
    }
}

task<> send_session(Socket sock) {
    char buf[buf_size];

    while (true) {
        int nw = co_await sock.send(buf);
        if (nw <= 0) {
            break;
        }
    }
}

task<> client(std::string_view hostname, uint16_t port) {
    inet_address addr;
    if (inet_address::resolve(hostname, port, addr)) {
        Socket sock{Socket::create_tcp(addr.family())};
        // 连接一个 server
        int res = co_await sock.connect(addr);
        if (res < 0) {
            printf("res=%d: %s\n", res, strerror(-res));
        }
        // 生成 session 协程
        co_spawn(recv_session(Socket{sock.fd()}));
        co_spawn(send_session(Socket{sock.fd()}));
    } else {
        printf("Unable to resolve %s\n", hostname.data());
    }
}

int main() {
    io_context ctx;

    ctx.co_spawn(client(hostname, port));

    ctx.run();

    return 0;
}
