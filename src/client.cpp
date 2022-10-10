#include "client.h"

#include "client_transport_plain.h"
#include "client_transport_tls.h"

Client::Client(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port, Request::Encryption encryption)
{
    // Setup encryption
    switch (encryption) {
        case Request::Encryption::NONE:
            transport = std::make_unique<ClientTransportPlain>(ioContext, host, port);
            break;

        case Request::Encryption::TLS:
            transport = std::make_unique<ClientTransportTLS>(ioContext, createSslContext(), host, port);
            transport->setHostname(host);
            break;
    }
}

boost::asio::ssl::context Client::createSslContext()
{
    // The SSL context is required, and holds certificates
    boost::asio::ssl::context sslContext(boost::asio::ssl::context::tlsv13_client);
    sslContext.set_default_verify_paths();

    // Verify the remote server's certificate
    sslContext.set_verify_mode(boost::asio::ssl::verify_peer);

    return sslContext;
}
