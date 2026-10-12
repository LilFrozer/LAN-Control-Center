
#pragma once

#include <string>
#include <cstdint>
#include "DevicesFinder/SubnetWorker.h"
#include "DevicesFinder/ArpWorker.h"

struct DeviceInfo {
    std::string subnet { "" };
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

class DeviceFinder : public std::enable_shared_from_this<DeviceFinder> {
protected:
    std::shared_ptr<ISubnetWorker> m_subnet_worker { nullptr };
    std::shared_ptr<IArpWorker> m_arp_worker { nullptr };
    PingMode m_ping_mode { PingMode::Icmp };
public:
    explicit DeviceFinder();
    std::vector<DeviceInfo> doScan( uint32_t timeout_ms = 2000 ) const;
};