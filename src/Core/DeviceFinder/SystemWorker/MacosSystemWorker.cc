#include "Core/DevicesFinder/SystemWorker/MacosSystemWorker.h"

#if defined(__APPLE__)

// -> Получение ipv4 подсетей на устройстве
std::vector<Ipv4SubnetInfo> MacosSystemWorker::doGetAllSubnets() const {
    auto flagsToString = []( const unsigned &flags ) -> std::string {
        std::string res{};

        auto checkEmpty = [&res]( const std::string &text ) -> void {
            res += text;
            if (!res.empty()) res += ";";
        };

        if (flags && IFF_UP) checkEmpty("UP");
        if (flags & IFF_BROADCAST) checkEmpty("BROADCAST");
        if (flags & IFF_DEBUG) checkEmpty("DEBUG");
        if (flags & IFF_LOOPBACK) checkEmpty("LOOPBACK");
        if (flags & IFF_POINTOPOINT) checkEmpty("POINTOPOINT");
        if (flags & IFF_RUNNING) checkEmpty("RUNNING");
        if (flags & IFF_NOARP) checkEmpty("NOARP");
        if (flags & IFF_PROMISC) checkEmpty("PROMISC");
        if (flags & IFF_ALLMULTI) checkEmpty("ALLMULTI");
        if (flags & IFF_MULTICAST) checkEmpty("MULTICAST");

        return res;
    };

    std::vector<Ipv4SubnetInfo> subnets{};

    struct ifaddrs* ifaddr{ nullptr };
    if (getifaddrs(&ifaddr) != 0) {
        std::cerr << "error -> Can`t get subnets!" << std::endl;
        return subnets;
    }

    for (auto ifa=ifaddr;ifa!=nullptr;ifa=ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        const auto family = ifa->ifa_addr->sa_family;
        const char* name = ifa->ifa_name;
        const auto flags = ifa->ifa_flags;

        if (family == AF_INET) {
            // -> если адрес ipv4
            char addr[INET_ADDRSTRLEN]{};
            char mask[INET_ADDRSTRLEN]{};

            // -> addr
            const auto* sin_addr = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            inet_ntop(AF_INET, &sin_addr->sin_addr, addr, sizeof(addr));

            // -> mask
            const auto* sin_mask = reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask);
            inet_ntop(AF_INET, &sin_mask->sin_addr, mask, sizeof(mask));

            subnets.emplace_back(Ipv4SubnetInfo{name,
                std::make_pair<unsigned, std::string>(ntohl(sin_addr->sin_addr.s_addr), addr),
                std::make_pair<unsigned, std::string>(static_cast<unsigned>(flags), flagsToString(flags)),
                std::make_pair<unsigned, std::string>(ntohl(sin_mask->sin_addr.s_addr), mask)});
        } else if (family == AF_INET6) {
            // -> если адрес ipv6
            char ip[INET6_ADDRSTRLEN] = {};
            const auto* sin6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
            inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(ip));
            std::cout << name << "  ip6=" << ip << std::endl;;
        } else {
            //
            std::cout << "..." << std::endl;
        }
    }

    freeifaddrs(ifaddr);

    return subnets;
}

// -> стук определенным способом по адресу "if_addr"
bool MacosSystemWorker::doPing( const PingMode &ping_mode, const uint32_t &if_addr, const uint32_t &timeout_ms ) {
    bool alive{false};

    switch (ping_mode) {
        case PingMode::Icmp: {

            break;
        }
        case PingMode::IcmpTerminal: {
            // in_addr addr{};
            // addr.s_addr = htonl(if_addr);
            // const std::string str_addr = inet_ntoa(addr);
            //
            // uint32_t timeoutSec = (timeout_ms + 999) / 1000;  // -> округляем вверх
            // if (timeoutSec < 1) timeoutSec = 1;
            //
            // std::string cmd{"ping -c 1 -W "};
            // cmd += std::to_string(timeout_ms);
            // cmd += " -t ";
            // cmd += std::to_string(timeoutSec);
            // cmd += " -q -n ";
            // cmd += str_addr;
            // cmd += " 2>&1";
            //
            // std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(cmd.c_str(), "r"), std::fclose);
            // if (!file) {
            //     logMessage("ERROR: Can`t execute cmd command!");
            //     return false;
            // }
            //
            // char line[512]{};
            // while (fgets(line, sizeof(line), file.get())) {
            //     std::string s(line);
            //     size_t pos = s.find("packets received");
            //     if (pos != std::string::npos) {
            //         // -> если нашли данную подстроку
            //         size_t comma = s.rfind(',', pos);
            //         if (comma != std::string::npos) {
            //             size_t numStart = s.find_first_of("0123456789", comma);
            //             if (numStart != std::string::npos && numStart < pos) {
            //                 int received = std::stoi(s.substr(numStart, pos - numStart));
            //                 alive = (received > 0);
            //             }
            //         }
            //     }
            // }

            break;
        }
        case PingMode::Tcp: {
            int socket{ ::socket(AF_INET, SOCK_STREAM, 0) };
            if (socket < 0) {
                logMessage("ERROR: socket<0...");
            } else {
                // -> неблокирующий режим, чтобы не ждать таймаут TCP по умолчанию
                int flags = fcntl(socket, F_GETFL, 0);
                fcntl(socket, F_SETFL, flags | O_NONBLOCK);

                sockaddr_in _addr{};
                _addr.sin_family = AF_INET;
                _addr.sin_port = htons(80);
                _addr.sin_addr.s_addr = htonl(if_addr);

                connect(socket, reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)); // -> вернёт EINPROGRESS сразу

                fd_set writeSet;
                FD_ZERO(&writeSet);
                FD_SET(socket, &writeSet);
                timeval tv{timeout_ms / 1000, static_cast<__darwin_suseconds_t>((timeout_ms % 1000) * 1000)};

                if (select(socket + 1, nullptr, &writeSet, nullptr, &tv) > 0) {
                    int err = 0;
                    socklen_t len = sizeof(err);
                    getsockopt(socket, SOL_SOCKET, SO_ERROR, &err, &len);
                    // -> ECONNREFUSED тоже значит "хост жив", просто порт закрыт
                    alive = (err == 0 || err == ECONNREFUSED);
                }

                close(socket);
            }
            break;
        }
        case PingMode::Udp: {

            break;
        }
        case PingMode::ArpRaw: {

            break;
        }
        case PingMode::NetBIOS: {

            break;
        }
        case PingMode::Mdns: {

            break;
        }
        case PingMode::Ssdp: {

            break;
        }
        default: {
            logMessage("ERROR: !PingMode::Icmp && !PingMode::Tcp80...");
            break;
        }
    }

    return alive;
}

std::unordered_map<std::string, std::string> MacosSystemWorker::doReadArp( const ArpGettingMode &mode ) const {
    std::unordered_map<std::string, std::string> addr_mac{};

    switch (mode) {
        case ArpGettingMode::Terminal: {
            // std::array<char, 128> buffer = {};
            //
            // std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("arp -an", "r"), pclose);
            // if (!pipe) {
            //     throw std::runtime_error("popen() failed");
            // }
            //
            // // * Читаем вывод
            // while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            //     std::string line(buffer.data());
            //     std::cout << line << std::endl;
            // }

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
