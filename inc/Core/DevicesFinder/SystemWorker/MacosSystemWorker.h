#pragma once

#include "SystemWorker.h"

#if defined(__APPLE__)

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if_dl.h>
#include <net/route.h>

class MacosSystemWorker final : public ISystemWorker, public std::enable_shared_from_this<MacosSystemWorker> {
public:
    ~MacosSystemWorker() override = default;
    bool doPing( const PingMode &ping_mode, const uint32_t &if_addr, const uint32_t &timeout_ms ) override;
    std::vector<Ipv4SubnetInfo> doGetAllSubnets() const override;
    std::unordered_map<std::string, std::string> doReadArp( const ArpGettingMode &mode ) const override;
};

#endif
