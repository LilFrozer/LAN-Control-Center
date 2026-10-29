#include "Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.h"

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
    const auto p = static_cast<MacosKeyboardWorker*>(ref);
    if (!p || !p->m_flag_run.load()) {
        return event;
    }

    if (type != kCGEventKeyDown) {
        return event;
    }

    /*в
        -> Что нажали, код клавиши + модификатор
    */
    CGKeyCode key_code = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = CGEventGetFlags(event);

    std::cout << key_code << "/" << flags << std::endl;


    // bool is_cmd_pressed = (flags & kCGEventFlagMaskCommand) != 0;
    // bool is_keyC_pressed = (key_code == 8);
    // if (is_keyC_pressed && is_cmd_pressed && type == kCGEventKeyDown) {
    //     ptr->signal_();
    // }

    return event;
}