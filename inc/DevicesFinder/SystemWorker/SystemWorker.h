#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <iostream>
#include <functional>

// -> данные ipv4 подсети текущего устр-ва
struct Ipv4SubnetInfo {
    std::string if_name{};
    using Options = std::pair<uint32_t, std::string>;
    Options if_addr{};
    Options if_flags{};
    Options if_mask{};
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

enum class ArpGettingMode : uint32_t {
    Terminal = 0,
    System
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
    // -> считываем таблицу arp после ping-а
    virtual std::unordered_map<std::string, std::string> doReadArp( const ArpGettingMode &mode ) const = 0;
};