#include "Network/Sender/TcpClient.h"
#include "Network/proto.h"

TcpClient::TcpClient( asio::io_context &ctx, const uint32_t &id ) :
    m_socket(ctx)
    , m_resolver(ctx)
    , m_current_client_id(id) {
}

TcpClient::~TcpClient() {
    if (m_socket.is_open()) {
        asio::error_code ec;
        m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close();
    }
}

void TcpClient::doRead() {
    const auto p_this = shared_from_this();
    asio::async_read(m_socket, asio::buffer(&m_rcv_size, sizeof(uint32_t)), [this, p_this]( asio::error_code ec, size_t ) {
        if (ec == asio::error::eof) {
            doDisconnect(m_current_client_id);
            return;
        }
        m_rcv_buffer.resize(m_rcv_size);
        asio::async_read(m_socket, asio::buffer(m_rcv_buffer), [this, p_this]( asio::error_code ec, size_t ) {
            if (ec == asio::error::eof) {
                doDisconnect(m_current_client_id);
                return;
            }
            // prcsPacket();
            doRead();
        });
    });
}

void TcpClient::doConnect( const std::string &ipv4, const uint16_t &port ) {
    m_resolver.async_resolve(ipv4, std::to_string(port), [this]( asio::error_code ec, asio::ip::tcp::resolver::results_type endpoints ) {
        if (ec) {
            doDisconnect(m_current_client_id);
            return;
        }
        asio::async_connect(m_socket, endpoints, [this]( asio::error_code ec, asio::ip::tcp::endpoint ) {
            if (ec) {
                doDisconnect(m_current_client_id);
            } else {
                std::cout << "Connected to server!" << std::endl;
                doRead();
            }
         });
    });
}

void TcpClient::doSend( const proto_project::Packet &packet ) {
    if (!m_socket.is_open()) {
        doDisconnect(m_current_client_id);
        return;
    }

    const std::vector<char> s{ proto_project::Packet::doSerialize(packet) };
    const uint32_t size = s.size();
    std::vector<asio::const_buffer> buffers;
    buffers.push_back(asio::buffer(&size, 4));
    buffers.push_back(asio::buffer(s));

    asio::async_write(m_socket, buffers, [this]( asio::error_code ec, size_t length ) {
        if (ec) {
            doDisconnect(m_current_client_id);
        }
    });
}