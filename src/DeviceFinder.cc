#include "DevicesFinder/DeviceFinder.h"
#include <iostream>

DeviceFinder::DeviceFinder() :
#if defined(__APPLE__)
    m_subnet_worker(std::make_shared<MacosSubnetWorker>())
#elif defined(WIN32)
    m_subnet_worker(std::make_shared<WindowsSubnetWorker>())
#elif defined(__linux__)

#else
    // -> неизвестная система
    #error ""
#endif
{

}

std::vector<DeviceInfo> DeviceFinder::doScan( uint32_t timeout_ms ) const
/**
 * -> Начинаем сканирование устр-в
 */
{
    std::vector<DeviceInfo> dev{};

    if (!m_subnet_worker) {
        std::cerr << "m_subnet_worker is nullptr!" << std::endl;
        return {};
    }

    const auto subnets{ m_subnet_worker->doGetAllSubnets() };
    for (const auto &subnet : subnets) {
        std::cout << subnet.if_name << "/" << subnet.if_addr.second << "/"
            << subnet.if_mask.second << "/" << subnet.if_flags.second << std::endl;
    }



    return dev;
}