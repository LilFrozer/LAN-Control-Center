
#pragma once

#include <string>
#include <cstdint>
#include "DevicesFinder/SystemWorker/MacosSystemWorker.h"
#include "DevicesFinder/SystemWorker/WindowsSystemWorker.h"
#include "DevicesFinder/SystemWorker/LinuxSystemWorker.h"
#include "DevicesFinder/ArpWorker/ArpWorker.h"
#include "DevicesFinder/ArpWorker/MacosArpWorker.h"
#include "DevicesFinder/VendorWorker/VendorWorker.h"
#include "DevicesFinder/VendorWorker/VendorWorkerCsv.h"
#include "DevicesFinder/VendorWorker/VendorWorkerDatabase.h"
#include <thread>
#include <iostream>
#include <asio.hpp>

struct DeviceInfo {
    std::string subnet{};
    std::string addr{};
    std::string mac{};
    std::string hostname{};
    std::string vendor{};
};

struct TimeoutDevicesWorker {
    bool is_active{false};
    std::unique_ptr<asio::steady_timer> timer{ nullptr };
    explicit TimeoutDevicesWorker( asio::io_context &ctx ) :
        timer(std::make_unique<asio::steady_timer>(ctx)) {}
};

class DevicesWorker : public std::enable_shared_from_this<DevicesWorker> {
protected:
    std::shared_ptr<ISystemWorker> m_system_worker{ nullptr };
    std::shared_ptr<IArpWorker> m_arp_worker{ nullptr };
    std::shared_ptr<IVendorWorker> m_vendor_worker{ nullptr };
public:
    explicit DevicesWorker();
    // -> выбираем, как будем определять vendor
    void chooseVendorWorker( const VendorMode &mode );
    // -> начинаем сканирование утср-в
    std::vector<DeviceInfo> doScan( uint32_t timeout_ms = 2000 ) const;
    // -> Сообщение в gui через систему сигнал+слот
    std::function<void( const std::string &msg )> logMessage;
};