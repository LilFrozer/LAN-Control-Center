#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <iostream>
#include <functional>

// -> данные ipv4 подсети текущего устр-ва
struct Ipv4SubnetInfo {
    std::string if_name{};
    using Param = std::pair<uint32_t, std::string>;
    Param if_addr{};
    Param if_flags{};
    Param if_mask{};
};

// -> режим сканирования устройства
enum class PingMode : uint32_t {
    Icmp = 0,               // -> Через сокет+select с протоколом icmp
    IcmpTerminal,           // -> Через системный вызов ping, потом парсим результат
    Tcp,                    // -> через сокет+select с протоколом tcp по 80 порту
    Udp,
    ArpRaw,
    NetBIOS,
    Mdns,
    Ssdp,
};

// -> интерфейс для системных вызовов
class ISystemWorker {
public:
    virtual ~ISystemWorker() = default;
    // -> Сообщение в gui через систему сигнал+слот
    std::function<void( const std::string &msg )> logMessage;
    // -> если обнаружено - true, в противном случае - false
    virtual bool doPing( const PingMode &ping_mode, const uint32_t &if_addr, const uint32_t &timeout_ms ) = 0;
    // -> поиск всех интерфейсов на устройстве
    virtual std::vector<Ipv4SubnetInfo> doGetAllSubnets() const = 0;
};