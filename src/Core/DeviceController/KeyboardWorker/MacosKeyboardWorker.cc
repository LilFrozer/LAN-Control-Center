#include "Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.h"

#if defined(__APPLE__)

void MacosKeyboardWorker::startScan() {
    if (m_flag_run.load()) {
        return;
    }
    m_flag_run.store(true);

    m_run_thread = std::thread([this]() {
        m_event_tap = CGEventTapCreate(
            kCGSessionEventTap,
            kCGHeadInsertEventTap,
            kCGEventTapOptionDefault,
            CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp),
            &MacosKeyboardWorker::systemScanning,
            this
        );

         if (!m_event_tap) {
            m_flag_run.store(false);
            return;
        }

        m_run_loop_src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, m_event_tap, 0);

        m_run_loop = CFRunLoopGetCurrent();
        CFRunLoopAddSource(m_run_loop, m_run_loop_src, kCFRunLoopCommonModes);

        while (m_flag_run.load()) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
        }

        // -> exit
        if (m_run_loop_src) {
            CFRunLoopRemoveSource(m_run_loop, m_run_loop_src, kCFRunLoopCommonModes);
            CFRelease(m_run_loop_src);
            m_run_loop_src = nullptr;
        }
        if (m_event_tap) {
            CFRelease(m_event_tap);
            m_event_tap = nullptr;
        }
        m_run_loop = nullptr;
    });
}

void MacosKeyboardWorker::stopScan() {
    m_flag_run.store(false);
    if (m_run_loop) {
        CFRunLoopWakeUp(m_run_loop);
    }
    if (m_run_thread.joinable()) {
        m_run_thread.join();
    }
}

CGEventRef MacosKeyboardWorker::systemScanning( CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *ref ) {
    const auto p_this = static_cast<MacosKeyboardWorker*>(ref);
    if (!p_this || !p_this->m_flag_run.load()) {
        return event;
    }

    static std::vector<CGKeyCode> buffer{};
    static std::chrono::steady_clock::time_point last_event{};
    static constexpr auto PAUSE_MS = std::chrono::milliseconds(50);

    if (type == kCGEventKeyDown) {
        // -> Что нажали, код клавиши + модификатор //
        CGKeyCode key_code{ static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode)) };
        const CGEventFlags flags{ CGEventGetFlags(event) };

        const auto now = std::chrono::steady_clock::now();

        if (!buffer.empty() && (now - last_event) >= PAUSE_MS) {
            KeyCombo combo{};
            combo.keys.reserve(buffer.size());
            for (const auto &key : buffer) {
                combo.keys.push_back(keyCodeToString(key));
            }
            combo.modifier = flagsToString(flags);
            // p_this->signalKeyPressed(combo);

            buffer.clear();
        }

        buffer.push_back(key_code);
        last_event = now;
    }

    if (p_this->m_block_input.load()) {
        return NULL;
    }

    return event;
}

std::string MacosKeyboardWorker::keyCodeToString( const CGKeyCode &code ) {
    static const std::unordered_map<CGKeyCode, std::string> map = {
        // буквы
        {0,  "A"}, {11, "B"}, {8,  "C"}, {2,  "D"}, {14, "E"},
        {3,  "F"}, {5,  "G"}, {4,  "H"}, {34, "I"}, {38, "J"},
        {40, "K"}, {37, "L"}, {46, "M"}, {45, "N"}, {31, "O"},
        {35, "P"}, {12, "Q"}, {15, "R"}, {1,  "S"}, {17, "T"},
        {32, "U"}, {9,  "V"}, {13, "W"}, {7,  "X"}, {16, "Y"},
        {6,  "Z"},
        // цифры
        {29, "0"}, {18, "1"}, {19, "2"}, {20, "3"}, {21, "4"},
        {23, "5"}, {22, "6"}, {26, "7"}, {28, "8"}, {25, "9"},
        // функциональные
        {122, "F1"}, {120, "F2"}, {99, "F3"},  {118, "F4"},
        {96,  "F5"}, {97,  "F6"}, {98,  "F7"},  {100, "F8"},
        {101, "F9"}, {109, "F10"},{103, "F11"}, {111, "F12"},
        // спецклавиши
        {36,  "Enter"}, {48, "Tab"}, {49, "Space"}, {51, "Backspace"},
        {53,  "Esc"},   {117, "Delete"}, {115, "Home"}, {119, "End"},
        {116, "PageUp"},{121, "PageDown"},
        {123, "Left"},  {124, "Right"}, {125, "Down"}, {126, "Up"},
        // модификаторы (на случай, если придёт KeyDown на них)
        {56,  "Shift"}, {60, "Shift"},   // left / right
        {59,  "Ctrl"},  {62, "Ctrl"},
        {58,  "Option"},{61, "Option"},
        {55,  "Cmd"},   {54, "Cmd"},
        {57,  "CapsLock"},
        // пунктуация
        {27, "-"}, {24, "="}, {33, "["}, {30, "]"}, {41, ";"},
        {39, "'"}, {43, ","}, {47, "."}, {44, "/"}, {42, "\\"},
        {50, "`"},
    };

    auto it = map.find(code);
    if (it != map.end()) return it->second;
    return "Key(" + std::to_string(code) + ")";
}

std::string MacosKeyboardWorker::flagsToString( const CGEventFlags &flags ) {
    std::string out;
    auto add = [&](const char* name) {
        if (!out.empty()) out += "+";
        out += name;
    };

    if (flags & kCGEventFlagMaskCommand)   add("Cmd");
    if (flags & kCGEventFlagMaskShift)     add("Shift");
    if (flags & kCGEventFlagMaskControl)   add("Ctrl");
    if (flags & kCGEventFlagMaskAlternate) add("Option");
    if (flags & kCGEventFlagMaskAlphaShift)add("CapsLock");

    if (out.empty()) out = "none";
    return out;
}

#endif