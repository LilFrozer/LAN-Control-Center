
#pragma once

#include <string>
#include <cstdint>
#include "DevicesFinder/SystemWorker/MacosSystemWorker.h"
#include "DevicesFinder/SystemWorker/WindowsSystemWorker.h"
#include "DevicesFinder/SystemWorker/LinuxSystemWorker.h"
#include "DevicesFinder/ArpWorker/ArpWorker.h"
#include "DevicesFinder/ArpWorker/MacosArpWorker.h"
#include <thread>
#include <iostream>

struct DeviceInfo {
    std::string subnet{};
    std::string addr{};
    std::string mac{};
    std::string hostname{};
    std::string vendor{};
};

class DevicesWorker : public std::enable_shared_from_this<DevicesWorker> {
protected:
    std::shared_ptr<ISystemWorker> m_system_worker{ nullptr };
    std::shared_ptr<IArpWorker> m_arp_worker{ nullptr };
public:
    explicit DevicesWorker();
    // -> начинаем сканирование утср-в
    std::vector<DeviceInfo> doScan( uint32_t timeout_ms = 2000 ) const;
    // -> Сообщение в gui через систему сигнал+слот
    std::function<void( const std::string &msg )> logMessage;
};