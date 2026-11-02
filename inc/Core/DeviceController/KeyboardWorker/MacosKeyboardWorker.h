#pragma once

#include "AbstractKeyboardWorker.h"
#include <memory>
#include <string>
#include <unordered_map>

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
    static std::string keyCodeToString( const CGKeyCode &code );
    static std::string flagsToString( const CGEventFlags &flags );
    std::function<void( const KeyCombo &combo )> signalKeyPressed;
    void slotKeyPressed(const KeyCombo &c) override;
};

#endif
