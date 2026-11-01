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


// struct LocalFinderDevicesTypes : public std::enable_shared_from_this<LocalFinderDevicesTypes> {
//     bool is_active = false;
//     std::unique_ptr<asio::steady_timer> timer{nullptr};
//     std::unique_ptr<IDeviceScaner> device_scaner{nullptr};
//     explicit LocalFinderDevicesTypes( asio::io_context &ctx, const std::string &path_db ) : 
//         timer(std::make_unique<asio::steady_timer>(ctx))
//         , device_scaner(std::make_unique<DeviceFinderScaner>("/Users/alekseypodoplelov/Documents/hyita01/etc/macvendor.db")) {}
// };

/**
 * -> Общий буфер обмена в локальной сети
 */
// struct LocalSharedClipboard : public std::enable_shared_from_this<LocalSharedClipboard> {
//     bool is_active = false;
//     std::shared_ptr<MulticastBus> multicast_bus{nullptr};
//     std::shared_ptr<IKeyScaner> key_scaner{nullptr};
//     explicit LocalSharedClipboard( asio::io_context &ctx ) : 
//         multicast_bus(std::make_shared<MulticastBus>(ctx))
//         , key_scaner(std::make_shared<MacKeyScaner>()) {}
// };

/**
 * -> Cканер входящего и выходящего трафика
 */
// struct LocalTraficMonitor : public std::enable_shared_from_this<LocalTraficMonitor> {
//     bool is_active = false;
//     std::shared_ptr<ITraficMonitor> trafic_monitor{nullptr};
//     explicit LocalTraficMonitor() : trafic_monitor(std::make_shared<MacosTraficMonitor>()) {}
// };

/**
 * -> Основной класс окна
 */
class Window final : public std::enable_shared_from_this<Window> {
    // -> поиск устр-в //
    std::unique_ptr<DeviceTable> m_devices_table{ nullptr };
    std::pair<std::unique_ptr<TimeoutDevicesWorker>, std::unique_ptr<DevicesWorker>> m_device_finder{};

    // -> управление устр-вом //
    std::unique_ptr<DeviceController> m_device_controller{ nullptr };

    // -> шина мультикаст //
    std::shared_ptr<MCWorker> m_mc_worker{ nullptr };
public:
    explicit Window( asio::io_context &ctx );
    ~Window();
    void draw();
    void doLocalScanningService();
    void doLocalClipboardService();
    void doTraficMonitorService();
};