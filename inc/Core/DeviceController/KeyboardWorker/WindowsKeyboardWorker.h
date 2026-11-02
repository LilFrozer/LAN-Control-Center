#pragma once

#include "AbstractKeyboardWorker.h"
#include <memory>

#if defined(_WIN32)

#include <windows.h>
#include <atomic>
#include <thread>
#include <algorithm>

class WindowsKeyboardWorker : public AbstractKeyboardWorker, public std::enable_shared_from_this<WindowsKeyboardWorker> {
public:
    WindowsKeyboardWorker() = default;
    ~WindowsKeyboardWorker() override {
        stopScan();
    }
    void startScan() override;
    void stopScan() override;
    void slotKeyPressed( const KeyCombo &c ) override;
private:
     // -> Флаг работы (аналог m_flag_run в macOS)
    std::atomic<bool> m_flag_run{false};

    // -> Поток, в котором работает message loop и хук
    std::thread m_run_thread;

    // -> Хэндл установленного хука
    HHOOK m_hook{nullptr};

    // -> Статический callback хука.
    //    Является статическим методом класса (как systemScanning в macOS),
    //    поэтому имеет доступ к private-полям без friend-объявлений.
    static LRESULT CALLBACK systemScanning(int nCode, WPARAM wParam, LPARAM lParam);

    WORD stringToVk(const std::string& s);
};

#endif