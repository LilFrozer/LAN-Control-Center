#pragma once 

#include "Core/DevicesFinder/DeviceFinder.h"
#include "Network/MCWorker.h"
#include <chrono>
#include <mutex>
#include "Core/DeviceController/DeviceController.h"

class DeviceTable {
public:
    void addDevice( const DeviceInfo& device );
    void clearTable();
    void drawTable( const std::unordered_map<std::string, std::chrono::steady_clock::time_point> &detected_devices ) const;
private:
    std::vector<DeviceInfo> m_devices_data;
    mutable std::mutex mutex_;
};

// -> окно //
class Window final : public std::enable_shared_from_this<Window> {
    // -> поиск устр-в //
    std::unique_ptr<DeviceTable> m_devices_table{ nullptr };
    std::pair<std::unique_ptr<TimeoutDevicesWorker>, std::unique_ptr<DevicesWorker>> m_device_finder{};

    // -> управление устр-вом //
    std::unique_ptr<DeviceController> m_device_controller{ nullptr };

    // -> мультикаст //
    std::shared_ptr<MCWorker> m_mc_worker{ nullptr };
public:
    explicit Window( asio::io_context &ctx );
    ~Window();
    void draw();
    void doLocalScanningService();
    void doLocalClipboardService();
    void doTraficMonitorService();
};