#pragma once

#include <memory>
#include "third_party/asio.hpp"
#include <cstdint>
#include <string>
#include <array>
#include <iostream>
#include "proto.h"
#include <unordered_map>

namespace multicast_namespace {
    const std::string MULTICAST_ADDR{"239.255.0.1"};
    constexpr uint16_t MULTICAST_PORT{5000};
}

class MCWorker : public std::enable_shared_from_this<MCWorker> {
    std::shared_ptr<asio::ip::udp::socket> m_socket{ nullptr };
    std::string m_current_ipv4{};
    asio::ip::udp::endpoint m_send_endpoint{};
    std::array<char, 1472> m_rcv_buf{};
    void handlePacket( const std::vector<char> &src, const std::string &ipv4 );

    std::shared_ptr<asio::steady_timer> m_timer_ping{ nullptr };
    void doPing();

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_detected_devices{};
    std::shared_ptr<asio::steady_timer> m_timer_cleanup{ nullptr };
    void doCleanup();
public:
    explicit MCWorker( asio::io_context &ctx, const std::string &ipv4 );
    ~MCWorker();
    void listen();
    void close() const;
    void doSend( const proto_project::Packet &packet ) const;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> getDetectedDevices() const;
};