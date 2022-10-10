#pragma once

#include <boost/asio/ssl/stream.hpp>

#include "client_transport.h"

class ClientTransportTLS final : public ClientTransport
{
public:
    ClientTransportTLS(boost::asio::io_context& ioContext, boost::asio::ssl::context& sslContext, const std::string& host, std::uint16_t port) :
        ClientTransport(ioContext, host, port),
        stream(ioContext, sslContext)
    {

    }

    boost::beast::tcp_stream& getStream() override { return stream.next_layer(); }

    boost::beast::error_code setHostname(const std::string& hostname) override
    {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str()))
            return boost::beast::error_code(static_cast<int>(ERR_get_error()), boost::asio::error::get_ssl_category());

        return {};
    }

    // void async_handshake() override
    // {
    //     stream.async_handshake(boost::asio::ssl::stream_base::client,
    //         [this](boost::beast::error_code error) {
    //             if (error)
    //                 return handleError(error);

    //             processQueue();
    //         });
    // }

    // virtual void write(boost::beast::http::request<boost::beast::http::string_body>& request) override
    // {
    //     boost::beast::http::write(stream, request);
    // }

    // virtual void read(boost::beast::http::response<boost::beast::http::dynamic_body>& response, boost::beast::error_code& error) override
    // {
    //     boost::beast::http::read(stream, buffer, response, error);
    // }

    // virtual void shutdown(boost::asio::ip::tcp::socket::shutdown_type type, boost::beast::error_code& error)
    // {
    //     stream.shutdown(error);

    //     ClientTransport::shutdown(type, error);
    // }

private:
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream;
};
