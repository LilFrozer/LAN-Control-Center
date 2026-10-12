
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

/* 
    ! Данные одной подсети 
*/
struct Ipv4SubnetInfo {
    std::string if_name{};
    std::pair<unsigned, std::string> if_addr{};
    std::pair<unsigned, std::string> if_flags{};
    std::pair<unsigned, std::string> if_mask{};
};

/* 
    ! Интерфейс, который позволяет найти все текущие подсети пк - проходимся по каждому сетевому устройству 
*/
class ISubnetWorker {
public:
    virtual ~ISubnetWorker() = default;
    virtual std::vector<Ipv4SubnetInfo> doGetAllSubnets() const = 0;
};

#if defined(__APPLE__)

/* 
    ! Для системы "Macos"
*/
class MacosSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<MacosSubnetWorker> {
public:
    std::vector<Ipv4SubnetInfo> doGetAllSubnets() const override;
};

#elif defined(_WIN32)

enum class ModeGettingSubnet : uint32_t {
    GetAdaptersAddresses = 0,
    GetIpAddrTable,
    WmiCom,
    WinSockApi
};

/* 
    ! Для системы "Windows"
*/
class WindowsSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<WindowsSubnetWorker> {
private:
    ModeGettingSubnet m_mode_getting_subnet{ ModeGettingSubnet::GetAdaptersAddresses };
public:
    std::vector<Subnet> doGetAllSubnets() const override;
    void set_mode_getting_subnet( const ModeGettingSubnet &mode );
    ModeGettingSubnet get_mode_getting_subnet() const;
};

#elif defined(__linux__)

/* 
    ! Для системы "Linux"
*/
class LinuxSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<LinuxSubnetWorker> {
public:
    std::vector<Subnet> doGetAllSubnets() const override;
};

#else 

#error "Unknown OS"

#endif
