#include "Core/DevicesFinder/DeviceFinder.h"

// -> Конструктор класса DevicesWorker, детектит операционную систему и соответствующий интерфейс
DevicesWorker::DevicesWorker() {
#if defined(__APPLE__)
    m_system_worker = std::make_shared<MacosSystemWorker>();
#elif defined(WIN32)
    m_system_worker = std::make_shared<WindowsSystemWorker>();
#elif defined(__linux__)
    // m_system_worker = std::make_shared<>();
#endif
}

// -> выбираем, как будем определять vendor
void DevicesWorker::chooseVendorWorker( const VendorMode &mode ) {
    if (!m_vendor_worker)
        m_vendor_worker.reset();

    switch (mode) {
        case VendorMode::Csv: {
            break;
        }
        case VendorMode::Database: {
            m_vendor_worker = std::make_shared<VendorWorkerDatabase>();
            // -> Обновляем данные с официальных источников при налачии интернет соединения
            m_vendor_worker->updateFilesFromOfficialSrc();
            // -> грузим дефолтную базу данных
            m_vendor_worker->loadFile("", true);
            break;
        }
        default: {

            break;
        }
    }
}

// -> начинаем сканирование устр-в
std::vector<DeviceInfo> DevicesWorker::doScan( uint32_t timeout_ms ) const {
    std::vector<DeviceInfo> devices{};

    if (!m_system_worker) {
        return {};
    }

    // -> 1. находим все подсети текущего устр-ва
    const auto subnets{ m_system_worker->doGetAllSubnets() };
    for (const auto &subnet : subnets) {
        // std::cout << subnet.if_name << "/" << subnet.if_addr.second << "/"
        //     << subnet.if_mask.second << "/" << subnet.if_flags.second << std::endl;

        // -> если не "en..", то скип
        if (subnet.if_name.find("en") == std::string::npos) {
            continue;
        }

        // -> 2. начинаем сканировать
        std::mutex devices_mutex;

        const uint32_t host_bits{~subnet.if_mask.first};
        const uint32_t net_addr{subnet.if_addr.first & subnet.if_mask.first};
        const uint32_t broadcast_addr{net_addr | host_bits};

        const uint64_t host_count = static_cast<uint64_t>(host_bits) + 1;
        constexpr uint64_t MAX_HOSTS = 1u << 16;   // 65 536
        if (host_count > MAX_HOSTS) {
            // -> пропускаем огромные подсети
            std::cerr << "ERROR: Skipping huge subnet: " << subnet.if_name
                      << " (" << host_count << " hosts)" << std::endl;
            continue;
        }

        std::vector<std::thread> workers{};
        std::atomic<uint32_t> next_ip{net_addr + 1};

        const uint32_t thread_count = std::max(1u, std::min(64u, std::thread::hardware_concurrency() * 8));

        for (auto i = 0; i < thread_count; ++i) {
            workers.emplace_back([&]() {
                uint32_t if_addr{0};
                while ((if_addr = next_ip.fetch_add(1)) < broadcast_addr) {
                    if (m_system_worker->doPing(PingMode::Tcp, if_addr, 300)) {
                        DeviceInfo device{};

                        sockaddr_in sa{};
                        sa.sin_family = AF_INET;
                        sa.sin_addr.s_addr = htonl(if_addr);

                        char host[NI_MAXHOST] = {};
                        const int ret = getnameinfo(reinterpret_cast<sockaddr*>(&sa), sizeof(sa),
                                            host, sizeof(host), nullptr, 0, NI_NAMEREQD);

                        // -> определяем имя хоста
                        if (ret == 0) {
                            device.hostname = host;
                        } else {
                            device.hostname = "NULL";
                        }

                        // -> определяем ipv4 адрес
                        char ipv4[INET_ADDRSTRLEN]{""};
                        inet_ntop(AF_INET, &sa.sin_addr, ipv4, sizeof(ipv4));
                        device.addr = ipv4;

                        // -> определяем какое сетевое устр-во обнаружило это устр-во
                        device.subnet = subnet.if_name + "/" +
                                        subnet.if_addr.second + "/" +
                                        subnet.if_mask.second + "/" +
                                        subnet.if_flags.second;

                        {
                            std::lock_guard<std::mutex> lock(devices_mutex);
                            devices.push_back(std::move(device));
                        }
                    }
                }
            });
        }

        for (auto& w : workers) {
            w.join();
        }
    }

    auto addr_mac{m_system_worker->doReadArp(ArpGettingMode::System)};
    for (auto &device : devices) {
        if (addr_mac.find(device.addr) != addr_mac.end()) {
            device.mac = addr_mac[device.addr];
        }
    }

    return devices;
}

std::vector<Ipv4SubnetInfo> DevicesWorker::doScanSubnets() const {
    const auto subnets{ m_system_worker->doGetAllSubnets() };
    return subnets;
}