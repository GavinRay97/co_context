#include "co_context/net/socket.hpp"

namespace co_context {

class inet_address;

socket &socket::bind(const inet_address &addr) {
    int res = ::bind(sockfd, addr.get_sockaddr(), addr.length());
    if (res != 0) {
        perror("socket::bind");
        abort();
    }

    return *this;
}

socket &socket::listen() {
    int res = ::listen(sockfd, SOMAXCONN);
    if (res != 0) {
        perror("socket::listen");
        abort();
    }

    return *this;
}

socket &socket::set_reuse_addr(bool on) {
    int optval = on;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)
        < 0) {
        perror("socket::setReuseAddr");
    }
    return *this;
}

socket &socket::set_tcp_no_delay(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(
            sockfd, IPPROTO_TCP, TCP_NODELAY, &optval,
            static_cast<socklen_t>(sizeof optval))
        < 0) {
        perror("socket::setTcpNoDelay");
    }
    return *this;
}

inet_address socket::get_local_addr() const {
    struct sockaddr_storage localaddr;
    socklen_t addrlen = sizeof localaddr;
    struct sockaddr *addr = reinterpret_cast<struct sockaddr *>(&localaddr);
    if (::getsockname(sockfd, addr, &addrlen) < 0) {
        perror("socket::getLocalAddr");
    }
    return inet_address(*addr);
}

inet_address socket::get_peer_addr() const {
    struct sockaddr_storage peeraddr;
    socklen_t addrlen = sizeof peeraddr;
    struct sockaddr *addr = reinterpret_cast<struct sockaddr *>(&peeraddr);
    if (::getpeername(sockfd, addr, &addrlen) < 0) {
        perror("socket::getPeerAddr");
    }
    return inet_address(*addr);
}

} // namespace co_context