
#pragma once

#include <string>
#include <cstdint>
#include "DevicesFinder/SubnerWorker.h"
#include "DevicesFinder/ArpWorker.h"

struct Device {
    std::string subnet { " " };
    std::string addr { "" };
    std::string mac { "" };
    std::string hostname { "" };
    std::string vendor { "" };
};

enum class PingMode : uint32_t {
    Icmp = 0,
    Tcp80,
    Arp
};

/* Модуль, сканирующий все подсети путем */
class DeviceFinder : public std::enable_shared_from_this<DeviceFinder> {
private:
    PingMode m_cur_ping_mode { PingMode::Tcp80 };
    bool doPing( const uint32_t &addr, const int32_t &timeout );
    std::vector<Device> doScanSubnet( const Subnet &subnet );
public:
    std::vector<Device> doScan();
};
