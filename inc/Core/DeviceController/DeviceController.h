#pragma once

#include "Core/DeviceController/KeyboardWorker/AbstractKeyboardWorker.h"
#include "Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.h"
#include "Core/DeviceController/KeyboardWorker/WindowsKeyboardWorker.h"

// -> При инициализации создает внутри себя TCP-клиент для приема и отправки //
// -> При инициализации создает внутри себя обработчик клавиатуры //
class DeviceController : public std::enable_shared_from_this<DeviceController> {
    std::shared_ptr<AbstractKeyboardWorker> m_kb_worker{ nullptr };
public:
    explicit DeviceController();
    ~DeviceController();
    void setActiveBlockKeyboard( const bool &f ) const;
    std::function<void( const KeyCombo &combo )> signalKeyPressed;
};