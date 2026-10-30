#pragma once

#include "SystemWorker.h"

#if defined(_WIN32)

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>

// Требуется для линковки
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

enum class ModeGetSubnet : uint8_t {
    GetAdaptersAddresses = 0,
    GetIpAddrTable,
    WmiCom,
    WinSockApi
};

class WindowsSystemWorker final : public ISystemWorker, public std::enable_shared_from_this<WindowsSystemWorker> {
    ModeGetSubnet m_mode_get_subnet{ ModeGetSubnet::GetAdaptersAddresses };
public:
    ~WindowsSystemWorker() override = default;
    bool doPing( const PingMode &ping_mode, const uint32_t &if_addr, const uint32_t &timeout_ms ) override;
    std::vector<Ipv4SubnetInfo> doGetAllSubnets() const override;
    std::unordered_map<std::string, std::string> doReadArp( const ArpGettingMode &mode ) const override;
};

#endif
