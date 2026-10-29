#include "UI/Window.h"
#include "imgui.h"

void DeviceTable::addDevice( const DeviceInfo& device ) {
    std::lock_guard<std::mutex> lock(mutex_);
    m_devices_data.push_back(device);
}

void DeviceTable::clearTable() {
    std::lock_guard<std::mutex> lock(mutex_);
    m_devices_data.clear();
}

void DeviceTable::  drawTable( const std::unordered_map<std::string, std::chrono::steady_clock::time_point> &detected_devices ) const {
    std::vector<DeviceInfo> devices_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        devices_copy = m_devices_data;
    }

    constexpr float MIN_CARD_WIDTH = 260.0f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const int cols = std::max(1, static_cast<int>(avail / (MIN_CARD_WIDTH + ImGui::GetStyle().ItemSpacing.x)));

    if (ImGui::BeginTable("##device_map", cols, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX)) {
        for (size_t i = 0; i < devices_copy.size(); ++i) {
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(i));

            // --- стилизация карточки ---
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // ← отступ текста от рамки
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));

            // ширина = 0 → занять всю ширину колонки (автомасштаб)
            if (ImGui::BeginChild("##card", ImVec2(0, 150), ImGuiChildFlags_AlwaysUseWindowPadding)) {
                if (detected_devices.find(devices_copy[i].addr) == detected_devices.end()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "OFFLINE");
                } else {
                    ImGui::TextColored(ImVec4(0.3f,0.9f,0.3f,1.0f), "ONLINE");
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(("-> " + devices_copy[i].addr).c_str());

                ImGui::Separator();

                ImGui::TextUnformatted("Подсеть ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].subnet.c_str());

                ImGui::Separator();

                ImGui::TextUnformatted("Имя ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].hostname.c_str());

                ImGui::Separator();

                ImGui::TextUnformatted("Mac ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].mac.c_str());

                ImGui::Separator();

                ImGui::TextUnformatted("Vendor ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].vendor.c_str());

                ImGui::Separator();

                if (ImGui::BeginTable("##metrics", 2, ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                    ImGui::TableSetupColumn("bar",   ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted("CPU");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::ProgressBar(0.2f, ImVec2(-FLT_MIN, 14), nullptr);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted("GPU");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::ProgressBar(0.2f, ImVec2(-FLT_MIN, 14), nullptr);

                    ImGui::EndTable();
                }

                ImGui::Spacing();

                // -> Кнопки
                if (ImGui::Button("Клавиатура", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {

                }
                // ImGui::SameLine();
                // if (ImGui::Button("", ImVec2(btn_w, 0)))     { /* ... */ }
                // ImGui::SameLine();
                // if (ImGui::Button("", ImVec2(btn_w, 0)))  { /* ... */ }
            }
            ImGui::EndChild();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

Window::Window( asio::io_context &ctx ) :
    m_devices_table(std::make_unique<DeviceTable>())
    , m_device_controller(std::make_unique<DeviceController>()) {
    m_device_finder.first = std::make_unique<TimeoutDevicesWorker>(ctx);
    m_device_finder.second = std::make_unique<DevicesWorker>();
    m_device_finder.second->chooseVendorWorker(VendorMode::Database);

    // -> ищем адрес для сокета под мультикаст //
    const auto subnets{ m_device_finder.second->doScanSubnets() };
    for (const auto &subnet : subnets) {
        if (subnet.if_name.find("en") == std::string::npos) {
            continue;
        }
        m_multicast_bus = std::make_unique<MulticastBus>(ctx, subnet.if_addr.second);
        break;
    }
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
        if (ImGui::BeginMenu("Настройки")) {
            // if (ImGui::MenuItem("Общий", "ВЫКЛЮЧЕН")) {
            //
            // }
            // if (ImGui::MenuItem("Мониторинг")) {
            //
            // }
            // if (ImGui::MenuItem("ICMP", "-")) {

            // }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextDisabled("Обнаружение устройств!");
    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    if (ImGui::BeginChild("##device_map", ImVec2(0, 260), ImGuiChildFlags_Borders)) {
        const auto &timer_object{ m_device_finder.first };
        if (ImGui::Button(timer_object->is_active ? "Остановить поиск!" : "Начать поиск!", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (timer_object->is_active) {
                timer_object->is_active = false;
                timer_object->timer->cancel();
            } else {
                doLocalScanningService();
            }
        }
        if (m_devices_table) {
            m_devices_table->drawTable(m_multicast_bus->getDetectedDevices());
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(); // цвет
    ImGui::PopStyleVar();   // толщина

    ImGui::Separator();

    ImGui::End();
}