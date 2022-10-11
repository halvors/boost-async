#pragma once

#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>

#include "client_transport.h"

class ClientTransportTLS final : public ClientTransport
{
public:
    explicit ClientTransportTLS(boost::asio::io_context& ioContext, boost::asio::ssl::context sslContext, const std::string& host, std::uint16_t port);

    boost::beast::error_code setHostname(const std::string& hostname) override;
    
    virtual void shutdown(boost::asio::ip::tcp::socket::shutdown_type type, const boost::beast::error_code& error);

protected:
    boost::beast::tcp_stream& getStream() override { return stream.next_layer(); }

    void asyncHandshake() override;
    void asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
    void asyncRead() override;

private:
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream;
};
