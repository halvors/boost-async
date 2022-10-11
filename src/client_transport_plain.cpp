#include "client_transport_plain.h"

ClientTransportPlain::ClientTransportPlain(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port) : 
    ClientTransport(ioContext, host, port),
    stream(ioContext)
{

}

void ClientTransportPlain::asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req)
{
    stream.expires_after(timeout);

    boost::beast::http::async_write(stream, req, 
        std::bind(&ClientTransportPlain::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
}

void ClientTransportPlain::asyncRead()
{
    stream.expires_after(timeout);

    boost::beast::http::async_read(stream, buffer, res, 
        std::bind(&ClientTransportPlain::handleRead, this, std::placeholders::_1, std::placeholders::_2));
}
