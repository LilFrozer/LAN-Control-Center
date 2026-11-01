#include "Core/DeviceController/DeviceController.h"

DeviceController::DeviceController() {
#if defined(__APPLE__)
    m_kb_worker = std::make_unique<MacosKeyboardWorker>();
    dynamic_cast<MacosKeyboardWorker*>(m_kb_worker.get())->signalKeyPressed = [this](const KeyCombo& c) {
        signalKeyPressed(c);
    };
#elif defined(WIN32)
    m_kb_worker = std::make_unique<WindowsKeyboardWorker>();
#elif defined(__linux__)

#endif

    m_kb_worker->startScan();
}

DeviceController::~DeviceController() {
    m_kb_worker->stopScan();
}

void DeviceController::setActiveBlockKeyboard( const bool &f ) const {
    m_kb_worker->setBlockInput(f);
}