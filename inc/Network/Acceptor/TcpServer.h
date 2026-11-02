#pragma once

#include "Network/Acceptor/TcpSession.h"

// -> общая прослушка //
class TcpServer : public std::enable_shared_from_this<TcpServer> {
    asio::ip::tcp::acceptor m_accept{ nullptr };
    void close();
public:
    explicit TcpServer( asio::io_context &ctx, const std::string &ipv4 );
    ~TcpServer();
    void listen();
    std::function<void( const KeyCombo &c )> rcvKeyCombo;
};