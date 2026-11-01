#include "Network/MulticastBus.h"

MulticastBus::MulticastBus( asio::io_context &ctx, const std::string &ipv4 ) :
    m_socket(std::make_shared<asio::ip::udp::socket>(ctx))
    , m_timer_ping(std::make_shared<asio::steady_timer>(ctx))
    , m_timer_cleanup(std::make_shared<asio::steady_timer>(ctx))
    , m_current_ipv4(ipv4)
{
    m_socket->open(asio::ip::udp::v4());
    m_socket->set_option(asio::ip::udp::socket::reuse_address(true));
    m_socket->bind(
        asio::ip::udp::endpoint(
            asio::ip::address_v4::any(),
            multicast_namespace::MULTICAST_PORT
        )
    );    
    m_socket->set_option(
        asio::ip::multicast::join_group(
            asio::ip::make_address_v4(multicast_namespace::MULTICAST_ADDR),
            asio::ip::make_address_v4(m_current_ipv4)
        )
    );
    m_socket->set_option(asio::ip::multicast::outbound_interface(asio::ip::make_address_v4(m_current_ipv4)));
    std::cout << "LOG: IPV4 MULTICAST -> " << m_current_ipv4 << std::endl;
    doPing();
    doCleanup();
}

void MulticastBus::doPing() {
    m_timer_ping->expires_after(std::chrono::seconds(1));
    m_timer_ping->async_wait([this](const asio::error_code ec) {
        if (ec == asio::error::operation_aborted) {
            return;
        }
        if (!ec) {
            proto_project::Packet hello{};
            hello.data_type = static_cast<uint8_t>(proto_project::DataType::PeerMulticast);
            doSend(hello);
            this->doPing();
        }
    });
}

void MulticastBus::doCleanup() {
    m_timer_cleanup->expires_after(std::chrono::seconds(5));
    m_timer_cleanup->async_wait([this]( const asio::error_code ec ) {
        if (ec == asio::error::operation_aborted) {
            return;
        }
        if (!ec) {
            auto now = std::chrono::steady_clock::now();
            for (auto it = m_detected_devices.begin(); it != m_detected_devices.end(); ) {
                const auto diff{ std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count() };
                if (diff > 5) {
                    std::cout << "LOG: DEVICE REMOVED, CUZ TIMEOUT: " << it->first << std::endl;
                    it = m_detected_devices.erase(it);
                } else {
                    ++it;
                }
            }
            this->doCleanup();
        }
    });
}

MulticastBus::~MulticastBus() {
    if (m_socket) {
        stopListen();
    }
}

void MulticastBus::startListen() {
    if (m_socket) {
        doReceive();
    }
}

void MulticastBus::stopListen() const {
    if (m_socket) {
        asio::error_code ec{};
        m_socket->close(ec);
    }
}

void MulticastBus::handlePacket( const std::vector<char> &src, const std::string &ipv4 ) {
    try {
        const auto packet{ proto_project::Packet::doDeserialise(src) };
        switch (static_cast<proto_project::DataType>(packet.data_type)) {
            case proto_project::DataType::PeerMulticast: {
                m_detected_devices[ipv4] = std::chrono::steady_clock::now();
                break;
            }
            default: {
                break;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void MulticastBus::doReceive() {
    auto self = shared_from_this();
    m_socket->async_receive_from(asio::buffer(m_rcv_buf), m_send_endpoint, [this, self](asio::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            const std::vector<char> data(m_rcv_buf.data(), m_rcv_buf.data() + bytes_transferred);
            const std::string sender_ipv4{ m_send_endpoint.address().to_string() };
            std::cout << "LOG: SENDER -> " << sender_ipv4 << " . CURRENT DEVICE -> " << m_current_ipv4 << std::endl;
            if (sender_ipv4 != m_current_ipv4) {
                handlePacket(data, sender_ipv4);
            }
            self->doReceive();
        } else {
            std::cerr << "ERROR: mc_rcv -> " << ec.message() << std::endl;
        }
    });
}

void MulticastBus::doSend( const proto_project::Packet &packet ) const {
    const auto src{ proto_project::Packet::doSerialize(packet) };
    const asio::ip::udp::endpoint multicast_endpoint(asio::ip::make_address(multicast_namespace::MULTICAST_ADDR), multicast_namespace::MULTICAST_PORT);
    m_socket->async_send_to(asio::buffer(src), multicast_endpoint, [](asio::error_code ec, std::size_t) {
        if (!ec) {

        } else {
            std::cerr << "ERROR: mc_send -> " << ec.message() << std::endl;
        }
    });
}

std::unordered_map<std::string, std::chrono::steady_clock::time_point> MulticastBus::getDetectedDevices() const {
    return m_detected_devices;
}