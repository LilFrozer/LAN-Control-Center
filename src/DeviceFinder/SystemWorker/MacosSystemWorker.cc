#include "DevicesFinder/SystemWorker/MacosSystemWorker.h"

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
            in_addr addr{};
            addr.s_addr = htonl(if_addr);
            const std::string str_addr = inet_ntoa(addr);

            uint32_t timeoutSec = (timeout_ms + 999) / 1000;  // -> округляем вверх
            if (timeoutSec < 1) timeoutSec = 1;

            std::string cmd{"ping -c 1 -W "};
            cmd += std::to_string(timeout_ms);
            cmd += " -t ";
            cmd += std::to_string(timeoutSec);
            cmd += " -q -n ";
            cmd += str_addr;
            cmd += " 2>&1";

            std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(cmd.c_str(), "r"), std::fclose);
            if (!file) {
                logMessage("ERROR: Can`t execute cmd command!");
                return false;
            }

            char line[512]{};
            while (fgets(line, sizeof(line), file.get())) {
                std::string s(line);
                size_t pos = s.find("packets received");
                if (pos != std::string::npos) {
                    // -> если нашли данную подстроку
                    size_t comma = s.rfind(',', pos);
                    if (comma != std::string::npos) {
                        size_t numStart = s.find_first_of("0123456789", comma);
                        if (numStart != std::string::npos && numStart < pos) {
                            int received = std::stoi(s.substr(numStart, pos - numStart));
                            alive = (received > 0);
                        }
                    }
                }
            }

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

#endif
