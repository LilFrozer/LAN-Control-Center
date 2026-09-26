
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

/* 
    ! Данные одной подсети 
*/
struct Subnet {
    uint32_t addr { 0 };
    uint32_t mask { 0 };
};

/* 
    ! Интерфейс, который позволяет найти все текущие подсети пк - проходимся по каждому сетевому устройству 
*/
class ISubnetWorker {
public:
    virtual ~ISubnetWorker() = default;
    virtual std::vector<Subnet> doGetAllSubnets() const = 0;
};

/* 
    ! Для системы "Macos"
*/
class MacosSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<MacosSubnetWorker> {
public:
    std::vector<Subnet> doGetAllSubnets() const override;
};

/* 
    ! Для системы "Windows"
*/
class WindowsSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<WindowsSubnetWorker> {
public:
    std::vector<Subnet> doGetAllSubnets() const override;
};

/* 
    ! Для системы "Linux"
*/
class LinuxSubnetWorker final : public ISubnetWorker, public std::enable_shared_from_this<LinuxSubnetWorker> {
public:
    std::vector<Subnet> doGetAllSubnets() const override;
};
