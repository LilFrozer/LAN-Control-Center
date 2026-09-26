#include "DevicesFinder/DeviceFinder.h"
#include <iostream>

void AbstractDeviceFinder::setPingMode( const PingMode &mode )
/**
 * 
 */
{
    m_ping_mode = mode;
}

PingMode AbstractDeviceFinder::getPingMode() const
/**
 * 
 */
{
    return m_ping_mode;
}

std::vector<DeviceInfo> AbstractDeviceFinder::doScan( uint32_t timeout_ms )
/**
 * -> Начинаем сканирование устр-в
 */
{
    if (!m_subnet_worker)
        std::cerr << "nullptr" << std::endl;
    // auto subnets{ m_subnet_worker->doGetAllSubnets() };
}

MacosDeviceFinder::MacosDeviceFinder()
/**
 * -> Конструктор для класса "Поиска устр-в в локальной сети" для системы MACOS
 */
{
    m_subnet_worker = std::make_shared<MacosSubnetWorker>();
    m_arp_worker = std::make_shared<MacosArpWorker>();
    std::cout << "MacosDeviceFinder()" << std::endl;
}