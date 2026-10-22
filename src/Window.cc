#include "Window.h"
#include "imgui.h"

void DeviceTable::addDevice( const DeviceInfo& device ) {
    std::lock_guard<std::mutex> lock(mutex_);
    m_devices_data.push_back(device);
}

void DeviceTable::clearTable() {
    std::lock_guard<std::mutex> lock(mutex_);
    m_devices_data.clear();
}

void DeviceTable::drawTable() const {
    std::vector<DeviceInfo> devices_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        devices_copy = m_devices_data;
    }

    // 2. Включаем флаг SizingFixedSame для корректного авторазмера колонок по тексту
    if (ImGui::BeginTable("##DATA", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame))
    {
        ImGui::TableSetupColumn("Локальная подсеть", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("IPV4", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Имя устройства", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("MAC", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("VENDOR", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableSetupColumn("ACTION", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (size_t i=0;i<devices_copy.size();++i) {
            const auto &device = devices_copy[i];
            ImGui::TableNextRow();

            ImGui::TableNextColumn(); ImGui::TextUnformatted(device.subnet.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(device.addr.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(device.hostname.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(device.mac.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(device.vendor.c_str());

            ImGui::TableNextColumn();
            std::string button_label = " Connect#" + std::to_string(i) + " ";

            if (ImGui::Button(button_label.c_str())) {
                // this->onDeviceActionTriggered(dev);
            }
        }

        ImGui::EndTable();
    }
}


void Window::onClipboardSlot()
/**
 * 
 */
{
    std::thread([this]() {
        std::cout << "Pressed!" << std::endl;
        // std::cout << "Text: " << local_clipboard_service_->key_scaner->get_text() << std::endl;
    }).detach();
}

Window::Window( asio::io_context &ctx ) :
    // m_device_finder(std::make_unique<LocalFinderDevicesTypes>(ctx, "/Users/alekseypodoplelov/Documents/hyita01/etc/macvendor.db"))
    m_devices_table(std::make_unique<DeviceTable>())
    // , local_clipboard_service_(std::make_shared<LocalSharedClipboard>(ctx))
    // , local_trafic_service_(std::make_shared<LocalTraficMonitor>())
/**
 * 
 */
{
    m_device_finder.first = std::make_unique<TimeoutDevicesWorker>(ctx);
    m_device_finder.second = std::make_unique<DevicesWorker>();
    m_device_finder.second->chooseVendorWorker(VendorMode::Database);
    // const auto devices{ m_device_finder.second->doScan() };
    // for (const auto &device : devices) {
    //     std::cout << device.subnet << "/"
    //     << device.addr << "/"
    //     << device.hostname << "/"
    //     << device.mac << "/"
    //     << device.vendor << "/"
    //     << std::endl;
    // }

    // local_clipboard_service_->key_scaner->setCallBack([this]() {
    //     onClipboardSlot();
    // });
}

void Window::doLocalScanningService() {
    const auto &timer_object = m_device_finder.first;
    if (!timer_object) return;

    timer_object->is_active = true;

    timer_object->timer->expires_after(std::chrono::seconds(2));
    timer_object->timer->async_wait([this](const asio::error_code ec) {
        auto &timer_obj = m_device_finder.first;
        auto &device_worker = m_device_finder.second;

        if (ec == asio::error::operation_aborted || !timer_obj->is_active) {
            timer_obj->is_active = false; // На всякий случай фиксируем статус
            return;
        }

        if (!ec) {
            const auto devices = device_worker->doScan();
            m_devices_table->clearTable();
            for (auto &i : devices) {
                m_devices_table->addDevice(i);
            }

            if (timer_obj->is_active) {
                this->doLocalScanningService();
            }
        }
    });
}


void Window::doLocalClipboardService()
/**
 * 
 */
{
    // if (local_clipboard_service_->is_active) {
    //     local_clipboard_service_->is_active = false;
    //     local_clipboard_service_->key_scaner->stopScan();
    //     local_clipboard_service_->multicast_bus->stopListen();
    //     return;
    // }
    // local_clipboard_service_->is_active = true;
    // local_clipboard_service_->key_scaner->startScan();
    // local_clipboard_service_->multicast_bus->startListen();
}

void Window::doTraficMonitorService()
/**
 * 
 */
{
    // if (local_trafic_service_->is_active) {
    //     local_trafic_service_->is_active = false;
    //     local_trafic_service_->trafic_monitor->stopCapture();
    //     return;
    // }
    // local_trafic_service_->is_active = true;
    // local_trafic_service_->trafic_monitor->startCapture();
}

void Window::draw()
/**
 * 
 */
{   
    auto *size = ImGui::GetMainViewport();          // -> Получаем размеры рабочей области приложения
    ImGui::SetNextWindowPos(size->WorkPos);         // -> Устанавливаем позицию в 0,0 относительно главного окна
    ImGui::SetNextWindowSize(size->WorkSize);       // -> Устанавливаем размер на всю ширину и высоту

    // -> Флаги окна
    auto window_flags = (ImGuiWindowFlags_MenuBar |
                        ImGuiWindowFlags_NoResize | 
                        ImGuiWindowFlags_NoMove | 
                        ImGuiWindowFlags_NoBringToFrontOnFocus | 
                        ImGuiWindowFlags_NoNavFocus |
                        ImGuiWindowFlags_NoTitleBar);

    bool fl = true;
    ImGui::Begin("LAN Control Center", &fl, window_flags);

    // -> Верхнее меню
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("-")) {
            // if (ImGui::MenuItem("TCP", "+")) {

            // }
            // if (ImGui::MenuItem("ARP", "-")) {

            // }
            // if (ImGui::MenuItem("ICMP", "-")) {

            // }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // ImGui::SetWindowSize(ImVec2(1024, 768), ImGuiCond_Once);

    const auto &timer_object = m_device_finder.first;
    if (ImGui::Button(timer_object->is_active ? " Остановить поиск! " : " Начать поиск! ")) {
        if (timer_object->is_active) {
            timer_object->is_active = false;
            timer_object->timer->cancel();
        } else {
            doLocalScanningService();
        }
    }
    ImGui::Separator();

    // if (ImGui::Button("Clipboard")) {
    //     doLocalClipboardService();
    // }
    // ImGui::SameLine();
    // if (ImGui::Button("TraficMonitor")) {
    //     doTraficMonitorService();
    // }
    // ImGui::Separator();

    if (m_devices_table) {
        m_devices_table->drawTable();
    }

    ImGui::End();
}