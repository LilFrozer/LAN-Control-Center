#include "Core/DeviceController/DeviceController.h"

DeviceController::DeviceController() {
#if defined(__APPLE__)
    m_kb_worker = std::make_unique<MacosKeyboardWorker>();
    static_cast<MacosKeyboardWorker*>(m_kb_worker.get())->sendKeyCombo = [this](const KeyCombo& c) {
        sendKeyCombo(c);
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

void DeviceController::rcvKeyCombo( const KeyCombo &combo ) {
    m_kb_worker->rcvKeyCombo(combo);
}