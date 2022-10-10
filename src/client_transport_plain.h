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

    boost::beast::tcp_stream& getStream() override { return stream; }

    void write(boost::beast::http::request<boost::beast::http::string_body>& req, HttpResponseHandler handler) override
    {
        // boost::beast::http::async_write(stream, req, 
        //     std::bind(&ClientTransportPlain::handle_write, this, std::placeholders::_1));

        boost::beast::http::async_write(stream, req, 
            [](boost::beast::error_code error, const size_t) {
                // if (error)
                //     return handleError(error);

                Log::error("Write works"); 
            });
    }

    // virtual void write(boost::beast::http::request<boost::beast::http::string_body>& request) override
    // {
    //     boost::beast::http::write(stream, request);
    // }

    // virtual void read(boost::beast::http::response<boost::beast::http::dynamic_body>& response, boost::beast::error_code& error) override
    // {
    //     boost::beast::http::read(stream, buffer, response, error);
    // }

private:
    boost::beast::tcp_stream stream;
};
