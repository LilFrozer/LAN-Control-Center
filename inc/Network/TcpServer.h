#pragma once

#include <memory>
#include <third_party/asio.hpp>

namespace tcp_const {
    constexpr uint16_t KB_PORT = 52;    // -> порт для управления клавиатурой //
}

class TcpServer : public std::enable_shared_from_this<TcpServer> {
    asio::ip::tcp::acceptor m_accept{ nullptr };

public:
    explicit TcpServer( asio::io_context &ctx, const std::string &ipv4 );
    ~TcpServer();
};