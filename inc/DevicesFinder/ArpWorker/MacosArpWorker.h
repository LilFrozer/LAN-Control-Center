#pragma once

#include "ArpWorker.h"

#if defined(__APPLE__)

class MacosArpWorker : public IArpWorker, public std::enable_shared_from_this<MacosArpWorker> {
public:
    ~MacosArpWorker() override = default;
    std::unordered_map<std::string, std::string> doReadArp( const ArpGettingMode &mode ) const override;
};

#endif
