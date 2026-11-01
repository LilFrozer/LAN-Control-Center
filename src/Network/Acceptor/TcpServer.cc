#include "Network/Acceptor/TcpServer.h"

TcpServer::TcpServer( asio::io_context &ctx, const std::string &ipv4 ) :
    m_accept(ctx) {
    const asio::ip::tcp::endpoint end_point{asio::ip::make_address_v4(ipv4), tcp_const::KB_PORT};
    m_accept.open(end_point.protocol());
    m_accept.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    m_accept.bind(end_point);
    m_accept.listen(asio::socket_base::max_listen_connections);
}

TcpServer::~TcpServer() {
    this->close();
}

void TcpServer::listen() {
    if (m_accept.is_open()) {
        const auto p_this{ shared_from_this() };
        m_accept.async_accept([p_this]( asio::error_code ec, asio::ip::tcp::socket socket ) {
            if (!ec) {
                const auto who_is { socket.remote_endpoint(ec) };
                if (!ec) {
                    auto session{std::make_shared<TcpSession>(socket)};
                    session->doRead();
                }
            }
            p_this->listen();
        }
        );
    }
}

void TcpServer::close() {
    asio::error_code ec;
    if (m_accept.is_open()) {
        m_accept.close(ec);
    }
}