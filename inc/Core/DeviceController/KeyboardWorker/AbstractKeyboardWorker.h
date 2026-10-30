#pragma once

#include <functional>
#include <thread>
#include <iostream>
#include <atomic>

class AbstractKeyboardWorker {
protected:
    std::thread m_run_thread;
    std::atomic<bool> m_flag_run{false};
public:
    virtual ~AbstractKeyboardWorker() = default;
    virtual void startScan() = 0;
    virtual void stopScan() = 0;
};