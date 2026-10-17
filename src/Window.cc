#include "Window.h"
#include "imgui.h"

// void DeviceTable::addRow(const DiscoveredDevice& device) 
// /**
//  * 
//  */
// {
//     std::lock_guard<std::mutex> lock(mutex_);
//     data_.push_back(device);
// }

void DeviceTable::clearTable() 
/**
 * 
 */
{
    std::lock_guard<std::mutex> lock(mutex_);
    // data_.clear();
}

size_t DeviceTable::getRowCount() const 
/**
 * 
 */
{
    // std::lock_guard<std::mutex> lock(mutex_);
    // return data_.size();
    return 0;
}

void DeviceTable::drawTable() 
/**
 * 
 */
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (ImGui::BeginTable("DevicesTable", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("IP", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Hostname", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("MAC", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Vendor", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // for (const auto& dev : data_) {
        //     ImGui::TableNextRow();
        //     ImGui::TableSetColumnIndex(0);
        //     ImGui::Text("%s", dev.addr.c_str());
        //     ImGui::TableSetColumnIndex(1);
        //     ImGui::Text("%s", dev.hostname.c_str());
        //     ImGui::TableSetColumnIndex(2);
        //     ImGui::Text("%s", dev.mac.c_str());
        //     ImGui::TableSetColumnIndex(3);
        //     ImGui::Text("%s", dev.vendor.c_str());
        // }

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
    m_devices_finder = std::make_shared<DevicesWorker>();
    m_devices_finder->chooseVendorWorker(VendorMode::Database);
    const auto devices{ m_devices_finder->doScan() };
    for (const auto &device : devices) {
        std::cout << device.subnet << "/"
        << device.addr << "/"
        << device.hostname << "/"
        << device.mac << "/"
        << device.vendor << "/"
        << std::endl;
    }

    // local_clipboard_service_->key_scaner->setCallBack([this]() {
    //     onClipboardSlot();
    // });
}

void Window::doLocalScanningService()
/**
 * 
 */
{
    // if (local_scanning_service_->is_active) {
    //     asio::error_code ec;
    //     local_scanning_service_->timer->cancel();
    //     local_scanning_service_->is_active = false;
    //     return;
    // } 
    // local_scanning_service_->is_active = true;
    // local_scanning_service_->timer->expires_after(std::chrono::seconds(3));
    // local_scanning_service_->timer->async_wait([this]( const asio::error_code ec) {
    //     local_scanning_service_->is_active = false;
    //     if (!ec) {
    //         std::vector<DiscoveredDevice> list_devices{local_scanning_service_->device_scaner->scanAllSubnet()};
    //         table_->clearTable();
    //         for (auto &i : list_devices) {
    //             table_->addRow(i);
    //         }
    //         this->doLocalScanningService();
    //     }
    // });
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

    if (ImGui::Button("doScan")) {
        doLocalScanningService();
    }
    // ImGui::Separator();
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