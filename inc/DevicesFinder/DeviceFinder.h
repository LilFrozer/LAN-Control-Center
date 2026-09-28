
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

class AbstractDeviceFinder {
/**
*/
protected:
    std::shared_ptr<ISubnetWorker> m_subnet_worker { nullptr };
    std::shared_ptr<IArpWorker> m_arp_worker { nullptr };
    PingMode m_ping_mode { PingMode::Tcp80 };
public:
    void setPingMode( const PingMode &mode );
    PingMode getPingMode() const;
    virtual ~AbstractDeviceFinder() = default;
    std::vector<DeviceInfo> doScan( uint32_t timeout_ms = 2000 );
};

#if defined(__APPLE__)

class MacosDeviceFinder : public AbstractDeviceFinder, public std::enable_shared_from_this<MacosDeviceFinder> {
private:

public:
    explicit MacosDeviceFinder();
    virtual ~MacosDeviceFinder() override = default;
};

#elif defined(_WIN32)

// class WindowsDeviceFinder : public AbstractDeviceFinder, public std::enable_shared_from_this<WindowsDeviceFinder> {
// /*
// */
// private:

// public:
//     
// };

#elif defined(__linux__)

#else

#error "Unknown OS"

#endif

// /* Модуль, сканирующий все подсети путем */
// class DeviceFinder : public std::enable_shared_from_this<DeviceFinder> {
// protected:
//     std::shared_ptr<ISubnetWorker> m_subnet_worker { nullptr };
//     std::shared_ptr<IArpWorker> m_arp_worker { nullptr };
// private:
//     PingMode m_cur_ping_mode { PingMode::Tcp80 };
//     bool doPing( const uint32_t &addr, const int32_t &timeout );
//     std::vector<Device> doScanSubnet( const Subnet &subnet );
// public:
//     std::vector<Device> doScan();
// };
