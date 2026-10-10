#include "DevicesFinder/SubnetWorker.h"

#if defined(__APPLE__)

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

std::vector<Subnet> MacosSubnetWorker::doGetAllSubnets() const
/**
 * 
 */
{
    std::vector<Subnet> subnets{};
    struct ifaddrs* ifaddr{ nullptr };
    if (getifaddrs(&ifaddr) != 0) {
        throw std::runtime_error("error -> can`t get local subnets!");
    }

    for (auto ifa=ifaddr;ifa!=nullptr;ifa=ifa->ifa_next) {
        auto *sin_addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        auto *sin_data = reinterpret_cast<sockaddr_in*>(ifa->ifa_data); 
        auto *sin_dstaddr = reinterpret_cast<sockaddr_in*>(ifa->ifa_dstaddr);
        auto *sin_flags = reinterpret_cast<sockaddr_in*>(ifa->ifa_flags);
        auto *sin_name = reinterpret_cast<sockaddr_in*>(ifa->ifa_name);
        auto *sin_mask = reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask);

        // std::cout << std::endl;
        // std::cout << sin_addr->sin_addr.s_addr << std::endl;
        // std::cout << sin_data->sin_addr.s_addr << std::endl;
        // std::cout << sin_dstaddr->sin_addr.s_addr << std::endl;
        // std::cout << sin_flags->sin_addr.s_addr << std::endl;
        // std::cout << sin_name->sin_addr.s_addr << std::endl;
        // std::cout << sin_mask->sin_addr.s_addr << std::endl;

        // unsigned addr = ntohl(sin_addr->sin_addr.s_addr);
        // unsigned mask = ntohl(sin_mask->sin_addr.s_addr);
    }

    freeifaddrs(ifaddr);

    std::cout << "MacosSubnetWorker::doGetAllSubnets()" << std::endl;
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