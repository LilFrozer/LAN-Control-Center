#include "DevicesFinder/SubnetWorker.h"

#if defined(__APPLE__)

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

std::vector<Ipv4SubnetInfo> MacosSubnetWorker::doGetAllSubnets() const
/**
 * 
 */
{
    auto flagsToString = []( const unsigned &flags ) -> std::string {
        std::string res{};

        auto checkEmpty = [&res]( const std::string &text ) -> void {
            res += text;
            if (!res.empty()) res += ";";
        };

        if (flags && IFF_UP) checkEmpty("UP");
        if (flags & IFF_BROADCAST) checkEmpty("BROADCAST");
        if (flags & IFF_DEBUG) checkEmpty("DEBUG");
        if (flags & IFF_LOOPBACK) checkEmpty("LOOPBACK");
        if (flags & IFF_POINTOPOINT) checkEmpty("POINTOPOINT");
        if (flags & IFF_RUNNING) checkEmpty("RUNNING");
        if (flags & IFF_NOARP) checkEmpty("NOARP");
        if (flags & IFF_PROMISC) checkEmpty("PROMISC");
        if (flags & IFF_ALLMULTI) checkEmpty("ALLMULTI");
        if (flags & IFF_MULTICAST) checkEmpty("MULTICAST");

        return res;
    };

    std::vector<Ipv4SubnetInfo> subnets{};

    struct ifaddrs* ifaddr{ nullptr };
    if (getifaddrs(&ifaddr) != 0) {
        throw std::runtime_error("error -> can`t get local subnets!");
    }

    for (auto ifa=ifaddr;ifa!=nullptr;ifa=ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        const auto family = ifa->ifa_addr->sa_family;
        const char* name = ifa->ifa_name;
        const auto flags = ifa->ifa_flags;

        if (family == AF_INET) {
            // -> если адрес ipv4
            char addr[INET_ADDRSTRLEN]{};
            char mask[INET_ADDRSTRLEN]{};

            // -> addr
            const auto* sin_addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            inet_ntop(AF_INET, &sin_addr->sin_addr, addr, sizeof(addr));

            // -> mask
            const auto* sin_mask = reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask);
            inet_ntop(AF_INET, &sin_mask->sin_addr, mask, sizeof(mask));

            subnets.emplace_back(Ipv4SubnetInfo{name,
                std::make_pair<unsigned, std::string>(ntohl(sin_addr->sin_addr.s_addr), addr),
                std::make_pair<unsigned, std::string>(static_cast<unsigned>(flags), flagsToString(flags)),
                std::make_pair<unsigned, std::string>(ntohl(sin_mask->sin_addr.s_addr), mask)});
        } else if (family == AF_INET6) {
            // -> если адрес ipv6
            // char ip[INET6_ADDRSTRLEN] = {};
            // auto* sin6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
            // inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(ip));
            // std::cout << name << "  ip6=" << ip << '\n';
        }
    }

    freeifaddrs(ifaddr);

    return subnets;
}

#elif defined(_WIN32)

void WindowsSubnetWorker::set_mode_getting_subnet( const ModeGettingSubnet &mode )
{
    this->m_mode_getting_subnet = mode;
}

ModeGettingSubnet WindowsSubnetWorker::get_mode_getting_subnet() const
{
    return this->m_mode_getting_subnet;
}

std::vector<Subnet> WindowsSubnetWorker::doGetAllSubnets() const
/**
 * 
 */
{
    switch (m_mode_getting_subnet) {
        case ModeGettingSubnet::GetAdaptersAddresses: {
            
            break;
        }
        case ModeGettingSubnet::GetIpAddrTable: {
            
            break;
        }
        case ModeGettingSubnet::WmiCom: {

            break;
        }
        case ModeGettingSubnet::WinSockApi: {

            break;
        }
    }
}

#elif defined(__linux__)

std::vector<Subnet> LinuxSubnetWorker::doGetAllSubnets() const
/**
 * 
 */
{
    
}

#else

#error "Unknown OS"

#endif