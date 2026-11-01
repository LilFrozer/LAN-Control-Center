#pragma once 

#include "Core/DevicesFinder/DeviceFinder.h"
#include "Network/MCWorker.h"
#include <chrono>
#include <mutex>
#include "Core/DeviceController/DeviceController.h"
#include "Network/Acceptor/TcpServer.h"
#include "Network/Sender/TcpClientManager.h"
#include "Network/proto.h"

// -> окно //
class Window final : public std::enable_shared_from_this<Window> {
    // -> поиск устр-в //
    std::shared_ptr<TimeoutDevicesWorker> m_timer_dvf{ nullptr };
    std::shared_ptr<DevicesWorker> m_worker_dvf{ nullptr };
    std::vector<DeviceInfo> m_found_dvf{};

    // -> управление устр-вом //
    std::shared_ptr<DeviceController> m_current_device_ctrl{ nullptr };

    // -> мультикаст //
    std::shared_ptr<MCWorker> m_mc_worker{ nullptr };

    // -> сервер для подключения + менеджер подключений //
    std::shared_ptr<TcpServer> m_tcp_server{ nullptr };
    std::shared_ptr<TcpClientManager> m_client_manager{ nullptr };
    asio::io_context *m_ctx{ nullptr };
public:
    explicit Window( asio::io_context &ctx );
    ~Window();
    void drawMainWindow();
    void drawDeviceTable();
    void doLocalScanningService();
};