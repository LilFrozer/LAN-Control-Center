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

void DeviceTable::drawTable( const std::unordered_map<std::string, std::chrono::steady_clock::time_point> &detected_devices ) const {
    std::vector<DeviceInfo> devices_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        devices_copy = m_devices_data;
    }

    constexpr float MIN_CARD_WIDTH = 260.0f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const int cols = std::max(1, static_cast<int>(avail / (MIN_CARD_WIDTH + ImGui::GetStyle().ItemSpacing.x)));

    if (ImGui::BeginTable("##device_map", cols, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX)) {
        if (devices_copy.empty()) {
            ImGui::TextDisabled("Нет данных об устройствах.");
        }
        for (size_t i = 0; i < devices_copy.size(); ++i) {
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(i));

            ImGui::PushStyleColor(ImGuiCol_Text, (detected_devices.find(devices_copy[i].addr) == detected_devices.end()) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.3f,0.9f,0.3f,1.0f));
            ImGui::TextUnformatted(devices_copy[i].subnet.c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(("-> " + devices_copy[i].addr).c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(("-> " + devices_copy[i].mac).c_str());
            ImGui::PopStyleColor();

            // --- стилизация карточки ---
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // ← отступ текста от рамки

            // ширина = 0 → занять всю ширину колонки (автомасштаб)
            if (ImGui::BeginChild("##card", ImVec2(0, 150), ImGuiChildFlags_AlwaysUseWindowPadding)) {
                ImGui::TextUnformatted("НАЗВАНИЕ ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].hostname.c_str());

                ImGui::TextUnformatted("ВЕНДОР ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(devices_copy[i].vendor.c_str());

                ImGui::Separator();

                // -> Кнопки
                // if (ImGui::Button("МЕТРИКИ", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                //
                // }
                if (ImGui::Button("КЛАВИАТУРА", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {

                }
                // ImGui::SameLine();
                // if (ImGui::Button("", ImVec2(btn_w, 0)))     { /* ... */ }
                // ImGui::SameLine();
                // if (ImGui::Button("", ImVec2(btn_w, 0)))  { /* ... */ }
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(3);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

Window::Window( asio::io_context &ctx ) :
    m_devices_table(std::make_unique<DeviceTable>())
    , m_device_controller(std::make_unique<DeviceController>()) 
{
    m_device_finder.first = std::make_unique<TimeoutDevicesWorker>(ctx);
    m_device_finder.second = std::make_unique<DevicesWorker>();
    m_device_finder.second->chooseVendorWorker(VendorMode::Database);

    // -> ищем адрес для сокета под мультикаст //
    const auto subnets{ m_device_finder.second->doScanSubnets() };
    for (const auto &subnet : subnets) {
        if (subnet.if_name.find("en") == std::string::npos) {
            continue;
        }
        m_mc_worker = std::make_shared<MCWorker>(ctx, subnet.if_addr.second);
        m_mc_worker->startListen();
        break;
    }
}

Window::~Window() {
    if (m_mc_worker) m_mc_worker->stopListen();
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

void Window::draw() {
    auto *size = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(size->WorkPos);
    ImGui::SetNextWindowSize(size->WorkSize);

    auto window_flags = (ImGuiWindowFlags_MenuBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoNavFocus |
                         ImGuiWindowFlags_NoTitleBar);

    // --- фон окна и отступы ---
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.10f, 0.12f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));

    bool fl = true;
    ImGui::Begin("LAN Control Center", &fl, window_flags);

    // ================= МЕНЮ =================
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 3));
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Настройки")) {
            // ImGui::MenuItem("Общий", "ВЫКЛЮЧЕН");
            // ImGui::MenuItem("Мониторинг");
            // ImGui::MenuItem("ICMP", "-");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::PopStyleVar();

    // ================= СТИЛИЗАЦИЯ ВКЛАДОК =================
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding,             8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize,        1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,            ImVec2(14, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,             ImVec2(6, 6));

    if (ImGui::BeginTabBar("##main_tabs",
                           ImGuiTabBarFlags_Reorderable |
                           ImGuiTabBarFlags_AutoSelectNewTabs |
                           ImGuiTabBarFlags_FittingPolicyScroll))
    {
        // ---------------- МОНИТОРИНГ ----------------
        if (ImGui::BeginTabItem("Мониторинг")) {
            ImGui::Spacing();
            ImGui::TextDisabled("Здесь будет мониторинг сети.");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextUnformatted("Пока пусто — добавьте виджеты позже.");
            ImGui::EndTabItem();
        }

        // ---------------- ОБНАРУЖЕНИЕ ----------------
        if (ImGui::BeginTabItem("Обнаружение")) {
            // ImGui::Spacing();

            // --- шапка вкладки: заголовок + кнопка поиска ---
            // ImGui::TextDisabled("Обнаружение устройств");
            // ImGui::SameLine();

            const auto &timer_object{ m_device_finder.first };
            const char *btn_label = timer_object->is_active ? "Остановить поиск" : "Начать поиск";

            // кнопка прижата вправо

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            if (ImGui::Button(btn_label, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                if (timer_object->is_active) {
                    timer_object->is_active = false;
                    timer_object->timer->cancel();
                } else {
                    doLocalScanningService();
                }
            }
            ImGui::PopStyleVar();

            // --- контент: растягиваем на всю оставшуюся высоту ---
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   ImVec2(10, 10));

            const ImVec2 avail = ImGui::GetContentRegionAvail();
            if (ImGui::BeginChild("##device_map", avail, ImGuiChildFlags_Borders)) {
                if (m_devices_table && m_mc_worker) {
                    m_devices_table->drawTable(m_mc_worker->getDetectedDevices());
                } else {
                    ImGui::TextDisabled("Нет данных об устройствах.");
                }
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(3);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // --- снимаем стили вкладок ---
    ImGui::PopStyleVar(4);

    ImGui::End();
    ImGui::PopStyleVar();  // WindowPadding
    ImGui::PopStyleColor(); // WindowBg
}