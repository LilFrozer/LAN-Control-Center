#include "Network/Acceptor/TcpSession.h"

TcpSession::TcpSession( decltype(m_socket) &socket ) : m_socket(std::move(socket)) {
    std::cout << "LOG: TcpSession!" << std::endl;
}

TcpSession::~TcpSession() {
    m_socket.close();
}

void TcpSession::doRead() {
    const auto ptr_this{ shared_from_this() };
    asio::async_read(m_socket, asio::buffer(&m_rcv_size, sizeof(uint32_t)), [this, ptr_this]( asio::error_code ec, size_t ) {
        if (ec == asio::error::eof) {
            std::cout << "LOG: DISCONNECTED!" << std::endl;
            return;
        }
        m_rcv_buffer.resize(m_rcv_size);
        asio::async_read(m_socket, asio::buffer(m_rcv_buffer), [this, ptr_this]( asio::error_code ec, size_t ) {
            if (ec) {
                std::cerr << "LOG: ERROR RCV!" << std::endl;
                return;
            }
            handlePacket();
            doRead();
        });
    });
}

void TcpSession::doWrite() {

}

void TcpSession::handlePacket() {
    proto_project::Packet packet = proto_project::Packet::doDeserialise(m_rcv_buffer);
    switch (static_cast<proto_project::DataType>(packet.data_type)) {
        case proto_project::DataType::KeyCombo: {
            rcvKeyCombo(KeyCombo::doDeserialize(packet.data));
            break;
        }
        default: {
            break;
        }
    }
}