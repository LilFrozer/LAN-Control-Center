#pragma once

#include "Core/DeviceController/KeyboardWorker/AbstractKeyboardWorker.h"
#include "Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.h"
#include "Core/DeviceController/KeyboardWorker/WindowsKeyboardWorker.h"

// -> При инициализации создает внутри себя TCP-клиент для приема и отправки //
// -> При инициализации создает внутри себя обработчик клавиатуры //
class DeviceController {
private:
    std::unique_ptr<AbstractKeyboardWorker> m_keyboard_worker{ nullptr };
public:
    explicit DeviceController();
    ~DeviceController();
};