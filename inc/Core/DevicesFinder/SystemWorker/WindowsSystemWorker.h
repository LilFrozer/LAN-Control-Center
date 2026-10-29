#pragma once

#include "SystemWorker.h"

#if defined(_WIN32)

// enum class ModeGettingSubnet : uint32_t {
//     GetAdaptersAddresses = 0,
//     GetIpAddrTable,
//     WmiCom,
//     WinSockApi
// };

// class WindowsSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<WindowsSubnetWorker> {
// private:
//     ModeGettingSubnet m_mode_getting_subnet{ ModeGettingSubnet::GetAdaptersAddresses };
// public:
//     std::vector<Subnet> doGetAllSubnets() const override;
//     void set_mode_getting_subnet( const ModeGettingSubnet &mode );
//     ModeGettingSubnet get_mode_getting_subnet() const;
// };

#endif
