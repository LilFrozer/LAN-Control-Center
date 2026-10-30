#include "Core/DeviceController/DeviceController.h"

DeviceController::DeviceController() {
#if defined(__APPLE__)
    m_keyboard_worker = std::make_unique<MacosKeyboardWorker>();
#elif defined(WIN32)
    m_keyboard_worker = std::make_unique<WindowsKeyboardWorker>();
#elif defined(__linux__)

#endif
    m_keyboard_worker->startScan();
}

DeviceController::~DeviceController() {
    m_keyboard_worker->stopScan();
}