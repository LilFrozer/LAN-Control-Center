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

    if (type == kCGEventKeyDown) {
        // -> Что нажали, код клавиши + модификатор //
        CGKeyCode key_code{ static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode)) };
        const CGEventFlags flags{ CGEventGetFlags(event) };

        KeyCombo combo{};
        combo.key = keyCodeToString(key_code);
        combo.modifier = flagsToString(flags);
        p_this->sendKeyCombo(combo);
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
        {36,  "enter"}, {48, "tab"}, {49, "space"}, {51, "backspace"},
        {53,  "esc"},   {117, "delete"}, {115, "home"}, {119, "end"},
        {116, "pageup"},{121, "pagedown"},
        {123, "left"},  {124, "right"}, {125, "down"}, {126, "up"},
        // модификаторы (на случай, если придёт KeyDown на них)
        {56,  "shift"}, {60, "shift"},   // left / right
        {59,  "ctrl"},  {62, "ctrl"},
        {58,  "option"},{61, "option"},
        {55,  "cmd"},   {54, "cmd"},
        {57,  "capslock"},
        // пунктуация
        {27, "-"}, {24, "="}, {33, "["}, {30, "]"}, {41, ";"},
        {39, "'"}, {43, ","}, {47, "."}, {44, "/"}, {42, "\\"},
        {50, "`"},
    };
    const auto it = map.find(code);
    if (it != map.end()) return it->second;
    return "";
}

std::string MacosKeyboardWorker::flagsToString( const CGEventFlags &flags ) {
    std::string out;
    auto add = [&](const char* name) {
        if (!out.empty()) out += "+";
        out += name;
    };

    if (flags & kCGEventFlagMaskCommand)    add("cmd");
    if (flags & kCGEventFlagMaskShift)      add("shift");
    if (flags & kCGEventFlagMaskControl)    add("ctrl");
    if (flags & kCGEventFlagMaskAlternate)  add("option");
    if (flags & kCGEventFlagMaskAlphaShift) add("capslock");

    return out;   // пустая строка, если нет модификаторов
}

void MacosKeyboardWorker::rcvKeyCombo(const KeyCombo &c) {

}

#endif