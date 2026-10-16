
#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

enum class ArpGettingMode : uint32_t {
    Terminal = 0,
    System
};

// -> интерфейс для работы с таблицей arp
class IArpWorker {
public:
    virtual ~IArpWorker() = default;
    std::function<void( const std::string &msg )> logMessage;
    virtual std::unordered_map<std::string, std::string> doReadArp( const ArpGettingMode &mode ) const = 0;
};