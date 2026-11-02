#include "Core/DeviceController/KeyboardWorker/WindowsKeyboardWorker.h"

#if defined(_WIN32)

#include <iostream>

// Статический указатель на экземпляр для доступа из статического callback
static WindowsKeyboardWorker* g_instance = nullptr;

LRESULT CALLBACK WindowsKeyboardWorker::systemScanning(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && g_instance && g_instance->m_flag_run.load()) {
        // wParam: WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, WM_SYSKEYUP
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            auto* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD vkCode = p->vkCode;

            // Получаем состояние модификаторов на момент события
            bool isCtrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool isShift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;
            bool isAlt   = (GetAsyncKeyState(VK_MENU)    & 0x8000) != 0;
            bool isWin   = ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) ||
                           ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0);

            // Формируем строку модификаторов для вывода (аналог flags в macOS)
            std::string mods;
            if (isCtrl)  mods += "Ctrl;";
            if (isShift) mods += "Shift;";
            if (isAlt)   mods += "Alt;";
            if (isWin)   mods += "Win;";

            // Вывод в стиле macOS: key_code / flags
            std::cout << vkCode << "/" << mods << std::endl;

            // Пример: перехват Ctrl+C (VK_C = 0x43)
            // if (isCtrl && vkCode == 0x43) {
            //     g_instance->signal_();
            // }
        }
    }

    // Передаём событие дальше
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void WindowsKeyboardWorker::startScan() {
    if (m_flag_run.load()) {
        return;
    }
    m_flag_run.store(true);
    g_instance = this;

    m_run_thread = std::thread([this]() {
        // Устанавливаем глобальный низкоуровневый хук клавиатуры
        m_hook = SetWindowsHookEx(
            WH_KEYBOARD_LL,
            &WindowsKeyboardWorker::systemScanning,
            GetModuleHandle(NULL),
            0
        );

        if (!m_hook) {
            std::cerr << "Failed to install keyboard hook. Error: " << GetLastError() << std::endl;
            m_flag_run.store(false);
            g_instance = nullptr;
            return;
        }

        // Цикл сообщений обязателен для работы хука
        MSG msg;
        while (m_flag_run.load()) {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    m_flag_run.store(false);
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(10);
        }

        // Завершение: снимаем хук
        if (m_hook) {
            UnhookWindowsHookEx(m_hook);
            m_hook = nullptr;
        }
        g_instance = nullptr;
    });
}

void WindowsKeyboardWorker::stopScan() {
    m_flag_run.store(false);
    if (m_run_thread.joinable()) {
        m_run_thread.join();
    }
}

// Вспомогательная функция: преобразование строки в виртуальный код клавиши Windows
WORD WindowsKeyboardWorker::stringToVk(const std::string& s) {
    if (s.empty()) return 0;
    
    // Одиночные символы (буквы и цифры)
    if (s.size() == 1) {
        char c = s[0];
        if (c >= 'a' && c <= 'z') return static_cast<WORD>(std::toupper(c));
        if (c >= 'A' && c <= 'Z') return static_cast<WORD>(c);
        if (c >= '0' && c <= '9') return static_cast<WORD>(c);
    }
    
    // Функциональные клавиши F1 - F12
    if (s.size() >= 2 && s[0] == 'F' && s[1] >= '1' && s[1] <= '9') {
        int num = std::stoi(s.substr(1));
        if (num >= 1 && num <= 12) return VK_F1 + (num - 1);
    }
    
    // Модификаторы и специальные клавиши (без учета регистра)
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "shift") return VK_SHIFT;
    if (lower == "ctrl" || lower == "control") return VK_CONTROL;
    if (lower == "alt" || lower == "option") return VK_MENU;
    if (lower == "cmd" || lower == "win" || lower == "command") return VK_LWIN;
    if (lower == "enter" || lower == "return") return VK_RETURN;
    if (lower == "tab") return VK_TAB;
    if (lower == "space") return VK_SPACE;
    if (lower == "esc" || lower == "escape") return VK_ESCAPE;
    if (lower == "backspace") return VK_BACK;
    if (lower == "delete" || lower == "del") return VK_DELETE;
    if (lower == "up") return VK_UP;
    if (lower == "down") return VK_DOWN;
    if (lower == "left") return VK_LEFT;
    if (lower == "right") return VK_RIGHT;
    
    return 0; // Неизвестная клавиша
}

void WindowsKeyboardWorker::slotKeyPressed(const KeyCombo &c) {
    std::vector<INPUT> inputs;
    
    // 1. Нажать модификаторы (например, Shift, Ctrl)
    WORD modVk = 0;
    if (!c.modifier.empty()) {
        // modifier может быть строкой, содержащей несколько модификаторов, соединенных знаком "+", например "Shift+Cmd"
        // Здесь обрабатывается только первый модификатор; для нескольких модификаторов требуется логика разделения
        modVk = stringToVk(c.modifier);
        if (modVk != 0) {
            INPUT in = {};
            in.type = INPUT_KEYBOARD;
            in.ki.wVk = modVk;
            inputs.push_back(in);
        }
    }
    
    // 2. Нажать основные клавиши (по порядку)
    for (const auto& keyStr : c.keys) {
        WORD vk = stringToVk(keyStr);
        if (vk == 0) continue;
        
        INPUT down = {};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = vk;
        inputs.push_back(down);
    }
    
    // 3. Отпустить основные клавиши (в обратном порядке)
    for (auto it = c.keys.rbegin(); it != c.keys.rend(); ++it) {
        WORD vk = stringToVk(*it);
        if (vk == 0) continue;
        
        INPUT up = {};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = vk;
        up.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(up);
    }
    
    // 4. Отпустить модификаторы
    if (modVk != 0) {
        INPUT up = {};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = modVk;
        up.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(up);
    }
    
    // 5. Отправить все события
    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    }
}

#endif // _WIN32