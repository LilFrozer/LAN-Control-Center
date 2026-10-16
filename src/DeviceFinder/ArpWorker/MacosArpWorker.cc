#include "../../../inc/DevicesFinder/ArpWorker/ArpWorker.h"
#include <iostream>

#if defined(__APPLE__)

#include <arpa/inet.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#include <net/route.h>
#include "DevicesFinder/ArpWorker/MacosArpWorker.h"

std::unordered_map<std::string, std::string> MacosArpWorker::doReadArp( const ArpGettingMode &mode ) const {
    std::unordered_map<std::string, std::string> addr_mac{};

    switch (mode) {
        case ArpGettingMode::Terminal: {
            std::array<char, 128> buffer = {};

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
        case ArpGettingMode::System: {
            int mib[6] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_LLINFO};
            size_t needed = 0;
            if (sysctl(mib, 6, nullptr, &needed, nullptr, 0) < 0) {
                logMessage("ERROR: sysctl < 0...");
                break;
            }

            std::vector<uint8_t> buf(needed);
            if (sysctl(mib, 6, buf.data(), &needed, nullptr, 0) < 0) {
                logMessage("ERROR: sysctl < 0...");
                break;
            }

            uint8_t* next = buf.data();
            uint8_t* end = buf.data() + needed;

            while (next < end) {
                auto rtm = reinterpret_cast<rt_msghdr*>(next);
                auto sin = reinterpret_cast<sockaddr_in*>(rtm + 1);
                auto sdl = reinterpret_cast<sockaddr_dl*>(reinterpret_cast<uint8_t*>(sin) + sin->sin_len);

                if (sdl->sdl_alen == 6) {
                    uint8_t* mac = reinterpret_cast<uint8_t*>(LLADDR(sdl));
                    char mac_str[18];
                    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                    sockaddr_in sa = *(reinterpret_cast<sockaddr_in*>(sin));
                    char addr_str[INET_ADDRSTRLEN]{""};
                    inet_ntop(AF_INET, &sa.sin_addr, addr_str, sizeof(addr_str));

                    // -> Находим адрес данного уст-ва и вставляем в структру mac
                    addr_mac[addr_str] = mac_str;
                }

                next += rtm->rtm_msglen;
            }
            break;
        }
    }

    return addr_mac;
}

#endif