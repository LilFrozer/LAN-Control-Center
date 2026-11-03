#pragma once

#include "Network/Sender/TcpClient.h"

// -> обработчик нескольких клиентов //
class TcpClientManager : public std::enable_shared_from_this<TcpClientManager> {
    std::unordered_map<uint32_t, std::pair<std::string, std::shared_ptr<TcpClient>>> m_clients{};
public:
    ~TcpClientManager();
    void addConnect( asio::io_context &ctx, const std::string &ipv4, const uint16_t &port );
    void removeConnect( const unsigned &id );
    void removeConnect( const std::string &ipv4 );
    void doSend( const proto_project::Packet &packet ) const;
    bool isClientConnected( const std::string &ipv4 ) const;
};
