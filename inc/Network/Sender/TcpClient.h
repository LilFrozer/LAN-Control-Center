#pragma once

#include <third_party/asio.hpp>
#include <iostream>
#include <functional>
#include <memory>
#include "Network/proto.h"
#include <unordered_map>

// -> делает только connect и send //
class TcpClient : public std::enable_shared_from_this<TcpClient> {
    asio::ip::tcp::socket m_socket{ nullptr };
    asio::ip::tcp::resolver m_resolver{ nullptr };
    uint32_t m_rcv_size{0};
    std::vector<char> m_rcv_buffer{};
    uint32_t m_current_client_id{0};
    void doRead();
public:
    explicit TcpClient( asio::io_context &ctx, const uint32_t &id );
    ~TcpClient();
    void doConnect( const std::string &ipv4, const uint16_t &port );
    void doSend( const proto_project::Packet &packet );
    std::function<void( const unsigned &id )> doDisconnect;
};