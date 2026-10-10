#include "DevicesFinder/DeviceFinder.h"
#include <iostream>

DeviceFinder::DeviceFinder() :
#if defined(__APPLE__)
    
#elif defined(WIN32)
    m_subnet_worker(std::make_shared<WindowsSubnetWorker>())
#elif defined(__linux__)

#else
    #error "unknown OS"
#endif
{

}

std::vector<DeviceInfo> DeviceFinder::doScan( uint32_t timeout_ms )
/**
 * -> Начинаем сканирование устр-в
 */
{
    if (!m_subnet_worker) {
        std::cerr << "m_subnet_worker is nullptr!" << std::endl;
        return;
    }

    auto subnets{ m_subnet_worker->doGetAllSubnets() };
}