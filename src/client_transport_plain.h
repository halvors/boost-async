#pragma once

#include "client_transport.h"

class ClientTransportPlain final : public ClientTransport
{
public:
    ClientTransportPlain(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port) : 
        ClientTransport(ioContext, host, port),
        stream(ioContext)
    {

    }

protected:
    boost::beast::tcp_stream& getStream() override { return stream; }

    void asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req) override
    {
        stream.expires_after(timeout);

        boost::beast::http::async_write(stream, req, 
            std::bind(&ClientTransportPlain::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
    }

    void asyncRead() override
    {
        stream.expires_after(timeout);

        boost::beast::http::async_read(stream, buffer, res, 
            std::bind(&ClientTransportPlain::handleRead, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    boost::beast::tcp_stream stream;
};
