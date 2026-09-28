
#pragma once

#include <memory>
#include <map>
#include <string>

/*
    ! Интерфейс для работы с arp
 */
class IArpWorker {
public:
    virtual ~IArpWorker() = default;
    virtual std::map<std::string, std::string> doReadArp() const = 0;
};

#if defined(__APPLE__)

enum class MacosArpMode : uint32_t {
    TerminalArpAN = 0,
    SysctlBSD
};

/* 
    ! Чтение таблицы Arp на системе "MacOS"
    ! Чтение может осуществляться с помощью двух способов: 1. Вызов из терминала "arp -an". 2. Используя системное sysctl()
*/
class MacosArpWorker final : public IArpWorker, public std::enable_shared_from_this<MacosArpWorker> {
private:
    MacosArpMode m_cur_mode { MacosArpMode::TerminalArpAN };
public:
    std::map<std::string, std::string> doReadArp() const override;
    void set_cur_mode( const MacosArpMode mode );
};

#elif defined(_WIN32)

/*
    ! Чтение таблицы Arp на системе "Windows"
*/
class WindowsArpWorker final : public IArpWorker, public std::enable_shared_from_this<WindowsArpWorker> {
public:
    std::map<std::string, std::string> doReadArp() const override;
};  

#elif defined(__linux__)

/*
    ! Чтение таблицы Arp на системе "Linux"
*/
class LinuxArpWorker final : public IArpWorker, public std::enable_shared_from_this<LinuxArpWorker> {
public:
    std::map<std::string, std::string> doReadArp() const override;
};

#else

#error "Unknown OS"

#endif