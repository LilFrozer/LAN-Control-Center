#pragma once

#include <memory>
#include <third_party/asio.hpp>
#include <iostream>
#include "Network/proto.h"
#include <functional>
#include "Core/DeviceController/DeviceController.h"

// -> сессия для одного //
class TcpSession : public std::enable_shared_from_this<TcpSession> {
    asio::ip::tcp::socket m_socket{ nullptr };
    uint32_t m_rcv_size{0};
    std::vector<char> m_rcv_buffer{};
    void handlePacket();
public:
    explicit TcpSession( decltype(m_socket) &socket );
    ~TcpSession();
    void doRead();
    void doWrite();
    std::function<void( const KeyCombo &c )> rcvKeyCombo;
};