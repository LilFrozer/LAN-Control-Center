#include "UI/Window.h"
#include "imgui.h"

Window::Window( asio::io_context &ctx ) :
    m_current_device_ctrl(std::make_shared<DeviceController>())
    , m_client_manager(std::make_shared<TcpClientManager>())
    , m_ctx(&ctx)
{
    m_timer_dvf = std::make_shared<TimeoutDevicesWorker>(ctx);
    m_worker_dvf = std::make_shared<DevicesWorker>();
    m_worker_dvf->chooseVendorWorker(VendorMode::Database);

    // -> ищем адрес для сокета под мультикаст //
    const auto subnets{ m_worker_dvf->doScanSubnets() };
    for (const auto &subnet : subnets) {
        if (subnet.if_name.find("en") == std::string::npos) {
            continue;
        }
        m_mc_worker = std::make_shared<MCWorker>(ctx, subnet.if_addr.second);
        m_mc_worker->listen();
        m_tcp_server = std::make_shared<TcpServer>(ctx, subnet.if_addr.second);
        m_tcp_server->listen();
        break;
    }

    m_current_device_ctrl->signalKeyPressed = [this]( const KeyCombo& c ) {
        proto_project::Packet packet{};
        packet.data_type = static_cast<uint8_t>(proto_project::DataType::KeyCombo);
        packet.data = KeyCombo::doSerialize(c);
        m_client_manager->doSend(packet);
    };
}

Window::~Window() {
    if (m_mc_worker) {
        m_mc_worker->close();
    }
}

void Window::doLocalScanningService() {
    if (!m_timer_dvf) {
        return;
    }

    m_timer_dvf->is_active = true;

    m_timer_dvf->timer->expires_after(std::chrono::seconds(2));
    m_timer_dvf->timer->async_wait([this](const asio::error_code ec) {
        if (ec == asio::error::operation_aborted || !m_timer_dvf->is_active) {
            m_timer_dvf->is_active = false; // На всякий случай фиксируем статус
            return;
        }
        if (!ec) {
            m_found_dvf = m_worker_dvf->doScan();
            if (m_timer_dvf->is_active) {
                this->doLocalScanningService();
            }
        }
    });
}

void Window::drawMainWindow() {
    const auto *size = ImGui::GetMainViewport();
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
            static bool flag_block_keyboard{ false };
            if (ImGui::MenuItem("Блокировка клавиатуры", (flag_block_keyboard) ? "ВКЛЮЧЕНА" : "ВЫКЛЮЧЕНА" )) {
                (flag_block_keyboard) ? flag_block_keyboard = false : flag_block_keyboard = true;
                m_current_device_ctrl->setActiveBlockKeyboard(flag_block_keyboard);
            }
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
            ImGui::EndTabItem();
        }

        // ---------------- ОБНАРУЖЕНИЕ ----------------
        if (ImGui::BeginTabItem("Обнаружение")) {
            const char *btn_label = m_timer_dvf->is_active ? "Остановить поиск" : "Начать поиск";
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            if (ImGui::Button(btn_label, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                if (m_timer_dvf->is_active) {
                    m_timer_dvf->is_active = false;
                    m_timer_dvf->timer->cancel();
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
                if (m_timer_dvf && m_worker_dvf && m_mc_worker) {
                    drawDeviceTable();
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

void Window::drawDeviceTable() {
    constexpr float MIN_CARD_WIDTH = 260.0f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const int cols = std::max(1, static_cast<int>(avail / (MIN_CARD_WIDTH + ImGui::GetStyle().ItemSpacing.x)));

    if (ImGui::BeginTable("##device_map", cols, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX)) {
        if (m_found_dvf.empty()) {
            ImGui::TextDisabled("Нет данных об устройствах.");
        }
        for (size_t i = 0; i < m_found_dvf.size(); ++i) {
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(i));

            ImGui::PushStyleColor(ImGuiCol_Text,
                (m_mc_worker->getDetectedDevices().find(m_found_dvf[i].addr) == m_mc_worker->getDetectedDevices().end()) ?
                ImVec4(1.0f, 0.0f, 0.0f, 1.0f) :
                ImVec4(0.3f,0.9f,0.3f,1.0f));
            ImGui::TextUnformatted(m_found_dvf[i].subnet.c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(("-> " + m_found_dvf[i].addr).c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(("-> " + m_found_dvf[i].mac).c_str());
            ImGui::PopStyleColor();

            // --- стилизация карточки ---
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // ← отступ текста от рамки

            // ширина = 0 → занять всю ширину колонки (автомасштаб)
            if (ImGui::BeginChild("##card", ImVec2(0, 150), ImGuiChildFlags_AlwaysUseWindowPadding)) {
                ImGui::TextUnformatted("НАЗВАНИЕ ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(m_found_dvf[i].hostname.c_str());

                ImGui::TextUnformatted("ВЕНДОР ->");
                ImGui::SameLine();
                ImGui::TextUnformatted(m_found_dvf[i].vendor.c_str());

                ImGui::Separator();

                // -> Кнопки
                // if (ImGui::Button("МЕТРИКИ", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                //
                // }
                if (ImGui::Button("КЛАВИАТУРА", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    m_client_manager->addConnect(*m_ctx, m_found_dvf[i].addr, tcp_const::KB_PORT);
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