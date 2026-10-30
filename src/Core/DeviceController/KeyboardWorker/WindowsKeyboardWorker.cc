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

#endif // _WIN32