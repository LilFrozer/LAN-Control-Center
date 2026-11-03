#include "Network/Sender/TcpClientManager.h"

void TcpClientManager::addConnect( asio::io_context &ctx, const std::string &ipv4, const uint16_t &port ) {
    static uint32_t id = 0;
    auto client{std::make_shared<TcpClient>(ctx, id)};
    m_clients[id] = std::move(client);
    m_clients[id]->doConnect(ipv4, port);
    m_clients[id]->doDisconnect = [this]( const unsigned &id ) {
        this->removeConnect(id);
    };
    std::cout << "LOG: Client`s added!" << std::endl;
    ++id;
}

TcpClientManager::~TcpClientManager() {
    m_clients.clear();
}

void TcpClientManager::removeConnect( const unsigned &id ) {
    m_clients.erase(m_clients.find(id));
    std::cout << "LOG: Client`s removed!" << std::endl;
}

void TcpClientManager::doSend( const proto_project::Packet &packet ) const {
    for (auto &i : m_clients) {
        i.second->doSend(packet);
    }
}