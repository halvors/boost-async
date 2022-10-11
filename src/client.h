#pragma once

#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/tcp_stream.hpp>

#include "client_transport.h"

class Client final
{
public:
    struct Request
    {
        enum class Encryption {
            NONE,
            TLS
        };

        HttpResponseHandler handler;

        // std::string host;
        // std::uint16_t port;
        // std::string target;
        // std::uint8_t http_version = 11;
        // Encryption encryption = Encryption::NONE;
    };

    struct Response
    {
        boost::beast::http::response<boost::beast::http::dynamic_body> raw;

        std::string body_string() const {
            return boost::beast::buffers_to_string(raw.body().data());
        }
    };

    explicit Client(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port, Request::Encryption encryption);

    boost::asio::ssl::context createSslContext();

    void send(boost::beast::http::verb method, const std::string& target, HttpResponseHandler handler, const nlohmann::json& json = {})
    {
        transport->enqueue(method, target, handler, json);
    }

// private:
    std::unique_ptr<ClientTransport> transport;
};
