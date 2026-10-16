#include "DevicesFinder/ArpWorker.h"
#include <iostream>

#if defined(__APPLE__)

std::map<std::string, std::string> MacosArpWorker::doReadArp() const 
/*
 * 
 */
{
    switch (m_cur_mode) {
        case MacosArpMode::TerminalArpAN: {
            std::array<char, 128> buffer = {};

            // * Используем uniq ptr, чтобы сразу же закрыть файл
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("arp -an", "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed");
            }

            // * Читаем вывод
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                std::string line(buffer.data());
                std::cout << line << std::endl;
            }

            break;
        }
        case MacosArpMode::SysctlBSD: {

            break;
        }
    }
}

void MacosArpWorker::set_cur_mode( const MacosArpMode mode )
/*
 * 
 */
{
    m_cur_mode = mode;
}

#elif defined(_WIN32)

std::map<std::string, std::string> WindowsArpWorker::doReadArp() const
/*
 *
*/
{

}

#elif defined(__linux__)

std::map<std::string, std::string> LinuxArpWorker::doReadArp() const
/*
 *
*/
{

}

#endif