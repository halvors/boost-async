#include "client_transport_tls.h"

#include "log.h"

ClientTransportTLS::ClientTransportTLS(boost::asio::io_context& ioContext, boost::asio::ssl::context sslContext, const std::string& host, std::uint16_t port) :
    ClientTransport(ioContext, host, port),
    stream(ioContext, sslContext)
{

}

boost::beast::error_code ClientTransportTLS::setHostname(const std::string& hostname)
{
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str()))
        return boost::beast::error_code(static_cast<int>(ERR_get_error()), boost::asio::error::get_ssl_category());

    return {};
}

void ClientTransportTLS::shutdown(boost::asio::ip::tcp::socket::shutdown_type, const boost::beast::error_code& error)
{
    stream.async_shutdown([](const boost::system::error_code&) {
        Log::info("SSL stream is now shut down");
    });

    //ClientTransport::shutdown(type, error);
}

void ClientTransportTLS::asyncHandshake()
{
    stream.async_handshake(boost::asio::ssl::stream_base::client,
        [this](const boost::beast::error_code error) {
            if (error)
                return handleError(error);

            processQueue();
        });
}

void ClientTransportTLS::asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req)
{
    boost::beast::get_lowest_layer(stream).expires_after(timeout);

    boost::beast::http::async_write(stream, req, 
        std::bind(&ClientTransportTLS::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
}

void ClientTransportTLS::asyncRead()
{
    boost::beast::get_lowest_layer(stream).expires_after(timeout);

    boost::beast::http::async_read(stream, buffer, res, 
        std::bind(&ClientTransportTLS::handleRead, this, std::placeholders::_1, std::placeholders::_2));
}
