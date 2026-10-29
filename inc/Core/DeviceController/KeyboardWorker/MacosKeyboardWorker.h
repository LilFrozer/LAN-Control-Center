#pragma once

#include "AbstractKeyboardWorker.h"
#include <memory>

#if defined(__APPLE__)

#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

class MacosKeyboardWorker : public AbstractKeyboardWorker, public std::enable_shared_from_this<MacosKeyboardWorker> {
    CFMachPortRef m_event_tap{ nullptr };
    CFRunLoopSourceRef m_run_loop_src{ nullptr };
    CFRunLoopRef m_run_loop{ nullptr };
public:
    void startScan() override;
    void stopScan() override;
    static CGEventRef systemScanning( CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *ref );
};

#endif
