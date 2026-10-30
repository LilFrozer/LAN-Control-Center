#include "Core/DevicesFinder/SystemWorker/WindowsSystemWorker.h"

#if defined(_WIN32)

// Вспомогательная функция для преобразования флагов (аналог macOS)
static std::string flagsToString(unsigned long flags) {
    std::string res;
    auto append = [&res](const std::string& s) {
        if (!res.empty()) res += ";";
        res += s;
    };

    if (flags & IP_ADAPTER_DHCP_ENABLED) append("DHCP");
    if (flags & IP_ADAPTER_RECEIVE_ONLY) append("RECEIVE_ONLY");
    if (flags & IP_ADAPTER_NO_MULTICAST) append("NO_MULTICAST");
    // Примечание: в Windows флаги адаптера отличаются от POSIX IFF_*.
    // Здесь приведены основные, но в Ipv4SubnetInfo лучше передавать сырое значение.
    
    if (res.empty()) res = "0";
    return res;
}

bool WindowsSystemWorker::doPing(const PingMode& ping_mode, const uint32_t& if_addr, const uint32_t& timeout_ms) {
    bool alive = false;

    // Инициализация Winsock (обычно должна быть вызвана один раз при старте приложения,
    // но для безопасности можно проверить/вызвать здесь, если это не гарантируется).
    // Если WSAStartup уже вызван в конструкторе класса, этот блок можно опустить.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logMessage("ERROR: WSAStartup failed in doPing");
        return false;
    }

    switch (ping_mode) {
        case PingMode::Icmp: {
            // Использование нативного Windows ICMP API
            HANDLE hIcmpFile = IcmpCreateFile();
            if (hIcmpFile == INVALID_HANDLE_VALUE) {
                logMessage("ERROR: IcmpCreateFile failed");
                break;
            }

            // Буфер для данных запроса
            char SendData[] = "Data Buffer";
            // Размер ответа: ICMP_ECHO_REPLY + размер данных (с запасом)
            DWORD ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + 8;
            std::vector<char> ReplyBuffer(ReplySize);

            // if_addr уже в host byte order (как в macOS версии), IcmpSendEcho ожидает IP в network order?
            // В документации IcmpSendEcho принимает IPAddr (DWORD) в network byte order (big-endian).
            // В вашей macOS версии if_addr приходит как ntohl(...). 
            // Для Windows необходимо преобразовать обратно в network order.
            IPAddr destAddr = htonl(if_addr);

            DWORD dwRetVal = IcmpSendEcho(
                hIcmpFile,
                destAddr,
                SendData,
                sizeof(SendData),
                nullptr,
                ReplyBuffer.data(),
                ReplySize,
                timeout_ms
            );

            if (dwRetVal != 0) {
                PICMP_ECHO_REPLY pEchoReply = reinterpret_cast<PICMP_ECHO_REPLY>(ReplyBuffer.data());
                // Если Status == IP_SUCCESS, значит хост ответил
                if (pEchoReply->Status == IP_SUCCESS) {
                    alive = true;
                }
            }

            IcmpCloseHandle(hIcmpFile);
            break;
        }
        case PingMode::IcmpTerminal: {
            // Реализация через вызов консоли (аналог закомментированного кода в macOS)
            // Для Windows можно использовать `ping -n 1 -w <timeout> <ip>`
            // Но лучше использовать нативный API, поэтому здесь оставим заглушку или 
            // реализуем через system()/CreateProcess, если действительно нужно.
            // В большинстве случаев достаточно PingMode::Icmp.
            logMessage("WARN: IcmpTerminal not implemented on Windows, falling back to Icmp.");
            return doPing(PingMode::Icmp, if_addr, timeout_ms);
        }
        case PingMode::Tcp: {
            // TCP-проверка (порт 80)
            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock == INVALID_SOCKET) {
                logMessage("ERROR: socket() failed in Tcp ping");
                break;
            }

            // Установка неблокирующего режима
            u_long mode = 1;
            if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
                logMessage("ERROR: ioctlsocket failed");
                closesocket(sock);
                break;
            }

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(80);
            addr.sin_addr.s_addr = htonl(if_addr); // Преобразуем host order в network order

            int res = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
            if (res == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    // Ожидание завершения подключения
                    fd_set writeSet, exceptSet;
                    FD_ZERO(&writeSet);
                    FD_ZERO(&exceptSet);
                    FD_SET(sock, &writeSet);
                    FD_SET(sock, &exceptSet);

                    timeval tv;
                    tv.tv_sec = timeout_ms / 1000;
                    tv.tv_usec = (timeout_ms % 1000) * 1000;

                    // На Windows ошибки подключения приходят в exceptfds
                    res = select(0, nullptr, &writeSet, &exceptSet, &tv);
                    if (res > 0) {
                        if (FD_ISSET(sock, &writeSet)) {
                            // Успешное подключение
                            alive = true;
                        } else if (FD_ISSET(sock, &exceptSet)) {
                            // Ошибка подключения
                            int optErr = 0;
                            int optLen = sizeof(optErr);
                            getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optErr), &optLen);
                            // ECONNREFUSED (WSAECONNREFUSED) означает, что хост доступен, но порт закрыт
                            if (optErr == WSAECONNREFUSED) {
                                alive = true;
                            }
                        }
                    }
                } else if (err == WSAECONNREFUSED) {
                    // Немедленный отказ (хост жив)
                    alive = true;
                }
            } else {
                // Подключение сразу удалось
                alive = true;
            }

            closesocket(sock);
            break;
        }
        case PingMode::Udp:
        case PingMode::ArpRaw:
        case PingMode::NetBIOS:
        case PingMode::Mdns:
        case PingMode::Ssdp: {
            // Заглушки для остальных режимов, аналогично macOS
            break;
        }
        default: {
            logMessage("ERROR: Unknown PingMode");
            break;
        }
    }

    // WSACleanup не вызываем здесь, так как WSAStartup должен быть парным на уровне приложения.
    return alive;
}

std::vector<Ipv4SubnetInfo> WindowsSystemWorker::doGetAllSubnets() const {
    std::vector<Ipv4SubnetInfo> subnets;

    switch (m_mode_get_subnet) {
        case ModeGetSubnet::GetAdaptersAddresses: {
            // Использование GetAdaptersAddresses (рекомендуемый способ)
            ULONG family = AF_INET; // Только IPv4
            ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
            DWORD dwSize = 15000; // Рекомендуемый начальный размер
            std::vector<BYTE> buffer(dwSize);
            PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

            DWORD dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &dwSize);
            if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
                buffer.resize(dwSize);
                pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
                dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &dwSize);
            }

            if (dwRetVal != NO_ERROR) {
                std::cerr << "GetAdaptersAddresses failed with error: " << dwRetVal << std::endl;
                break;
            }

            for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
                // Пропускаем неактивные адаптеры (опционально)
                if (pCurrAddresses->OperStatus != IfOperStatusUp) continue;

                // Имя адаптера (FriendlyName)
                std::wstring wname = pCurrAddresses->FriendlyName;
                std::string name(wname.begin(), wname.end()); // Упрощённое 
                if (wname == L"Ethernet" ||
                    wname == L"Wi-Fi" ||
                    wname == L"Беспроводная сеть")
                {
                    name = "en0";
                }

                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {
                    sockaddr* sa = pUnicast->Address.lpSockaddr;
                    if (sa->sa_family == AF_INET) {
                        sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(sa);
                        char addrStr[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &sin->sin_addr, addrStr, sizeof(addrStr));

                        // Маска хранится в OnLinkPrefixLength (длина префикса)
                        ULONG mask = 0;
                        ConvertLengthToIpv4Mask(pUnicast->OnLinkPrefixLength, &mask);
                        // ConvertLengthToIpv4Mask возвращает маску в network byte order? 
                        // Согласно документации, результат в network byte order (big-endian).
                        // Для совместимости с Ipv4SubnetInfo (где ожидается unsigned в host order? В macOS использовался ntohl),
                        // преобразуем.
                        unsigned maskHost = ntohl(mask);
                        
                        char maskStr[INET_ADDRSTRLEN];
                        in_addr maskAddr;
                        maskAddr.s_addr = mask; // mask уже в network order
                        inet_ntop(AF_INET, &maskAddr, maskStr, sizeof(maskStr));

                        // Флаги адаптера (можно передать сырое значение, но семантика отличается)
                        unsigned flagsVal = 0; // В Windows флаги адаптера не совпадают с IFF_*, можно оставить 0 или маппить.
                        
                        subnets.emplace_back(Ipv4SubnetInfo{
                            name,
                            std::make_pair<unsigned, std::string>(ntohl(sin->sin_addr.s_addr), addrStr),
                            std::make_pair<unsigned, std::string>(static_cast<unsigned>(flagsVal), flagsToString(flagsVal)),
                            std::make_pair<unsigned, std::string>(static_cast<unsigned>(maskHost), maskStr)
                        });
                    }
                }
            }
            break;
        }
        case ModeGetSubnet::GetIpAddrTable: {
            // Альтернативный метод через GetIpAddrTable
            PMIB_IPADDRTABLE pIPAddrTable = nullptr;
            DWORD dwSize = 0;
            DWORD dwRetVal = 0;

            // Первый вызов для получения размера
            if (GetIpAddrTable(nullptr, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
                pIPAddrTable = reinterpret_cast<PMIB_IPADDRTABLE>(malloc(dwSize));
            }

            if (!pIPAddrTable) {
                logMessage("ERROR: malloc failed for GetIpAddrTable");
                break;
            }

            if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, FALSE)) != NO_ERROR) {
                logMessage("ERROR: GetIpAddrTable failed");
                free(pIPAddrTable);
                break;
            }

            for (DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++) {
                MIB_IPADDRROW& row = pIPAddrTable->table[i];
                // dwAddr и dwMask возвращаются в host byte order (согласно документации)
                in_addr addr, mask;
                addr.s_addr = htonl(row.dwAddr); // Для inet_ntop нужен network order
                mask.s_addr = htonl(row.dwMask);

                char addrStr[INET_ADDRSTRLEN];
                char maskStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr, addrStr, sizeof(addrStr));
                inet_ntop(AF_INET, &mask, maskStr, sizeof(maskStr));

                // Имя интерфейса не возвращается напрямую в GetIpAddrTable, 
                // можно использовать dwIndex для связи с именем через другие API, 
                // но здесь оставим пустым или используем индекс как строку.
                std::string name = "ifIndex_" + std::to_string(row.dwIndex);

                subnets.emplace_back(Ipv4SubnetInfo{
                    name,
                    std::make_pair<unsigned, std::string>(row.dwAddr, addrStr), // host order
                    std::make_pair<unsigned, std::string>(0, "0"), // флаги недоступны
                    std::make_pair<unsigned, std::string>(row.dwMask, maskStr)
                });
            }

            free(pIPAddrTable);
            break;
        }
        case ModeGetSubnet::WmiCom: {
            // Заглушка (требует COM и WMI, выходит за рамки простого примера)
            logMessage("WARN: WmiCom mode not implemented.");
            break;
        }
        case ModeGetSubnet::WinSockApi: {
            // Заглушка (можно реализовать через gethostname + getaddrinfo, но это не даст маски)
            logMessage("WARN: WinSockApi mode not implemented.");
            break;
        }
    }

    return subnets;
}

std::unordered_map<std::string, std::string> WindowsSystemWorker::doReadArp(const ArpGettingMode& mode) const {
    std::unordered_map<std::string, std::string> addr_mac;

    switch (mode) {
        case ArpGettingMode::Terminal: {
            // Аналог `arp -a` через консоль (можно реализовать, но лучше использовать System API)
            logMessage("WARN: Terminal ARP mode not implemented, falling back to System.");
            return doReadArp(ArpGettingMode::System);
        }
        case ArpGettingMode::System: {
            // Использование GetIpNetTable
            PMIB_IPNETTABLE pIpNetTable = nullptr;
            ULONG size = 0;
            DWORD dwRetVal = 0;

            // Первый вызов для получения размера
            if (GetIpNetTable(nullptr, &size, TRUE) == ERROR_INSUFFICIENT_BUFFER) {
                pIpNetTable = reinterpret_cast<PMIB_IPNETTABLE>(malloc(size));
            }

            if (!pIpNetTable) {
                logMessage("ERROR: malloc failed for GetIpNetTable");
                break;
            }

            if ((dwRetVal = GetIpNetTable(pIpNetTable, &size, TRUE)) != NO_ERROR) {
                if (dwRetVal != ERROR_NO_DATA) {
                    logMessage("ERROR: GetIpNetTable failed");
                }
                free(pIpNetTable);
                break;
            }

            for (DWORD i = 0; i < pIpNetTable->dwNumEntries; i++) {
                MIB_IPNETROW& row = pIpNetTable->table[i];
                // Пропускаем записи с недействительным MAC (например, broadcast)
                if (row.dwPhysAddrLen != 6) continue;

                // IP-адрес (dwAddr в network byte order? Согласно документации, dwAddr в network byte order)
                in_addr addr;
                addr.s_addr = row.dwAddr; // Уже network order?
                // В документации GetIpNetTable: dwAddr в network byte order (big-endian).
                // Проверим: обычно возвращается в network order.
                char addrStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr, addrStr, sizeof(addrStr));

                // MAC-адрес
                char macStr[18];
                snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                         row.bPhysAddr[0], row.bPhysAddr[1], row.bPhysAddr[2],
                         row.bPhysAddr[3], row.bPhysAddr[4], row.bPhysAddr[5]);

                addr_mac[addrStr] = macStr;
            }

            free(pIpNetTable);
            break;
        }
    }

    return addr_mac;
}

#endif