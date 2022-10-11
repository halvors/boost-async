#pragma once

#include "client_transport.h"

class ClientTransportPlain final : public ClientTransport
{
public:
    explicit ClientTransportPlain(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port);

    ~ClientTransportPlain()
    {
        // boost::beast::error_code ec;
        // stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // Log::info("test1");

        // if (ec)
        //     Log::error(fmt::format("test2: {} ({})", ec.message(), ec.value()));

        // Log::info("test2");
    }

protected:
    boost::beast::tcp_stream& getStream() override { return stream; }

    void asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req) override;
    void asyncRead() override;

private:
    boost::beast::tcp_stream stream;
};
