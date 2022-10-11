#pragma once

#include <chrono>
#include <deque>

#include <boost/asio/ip/resolver_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>

using namespace std::literals::chrono_literals;

typedef std::function<void(boost::beast::http::status, const nlohmann::json&)> HttpResponseHandler;

class ClientTransport
{
public:
    explicit ClientTransport(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port);
    virtual ~ClientTransport() = default;

    virtual boost::beast::error_code setHostname([[maybe_unused]] const std::string& hostname) { return {}; }

    void enqueue(boost::beast::http::verb method, const std::string& target, HttpResponseHandler handler, const nlohmann::json& json);
    virtual void shutdown(boost::asio::ip::tcp::socket::shutdown_type type, boost::beast::error_code& error);

protected:
    struct Request
    {
        boost::beast::http::request<boost::beast::http::string_body> req;
        HttpResponseHandler handler;
    };

    virtual boost::beast::tcp_stream& getStream() = 0;
    virtual void asyncHandshake() { processQueue(); }
    virtual void asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req) = 0;
    virtual void asyncRead() = 0;

    void handleWrite(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/);
    void handleRead(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/);
    void processQueue();

    static void handleError(boost::system::error_code error);

    boost::asio::ip::tcp::resolver::query query;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::flat_buffer buffer;

    std::deque<Request> queue;
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    bool running = false;

    static constexpr std::chrono::seconds timeout = 30s;

private:
    static std::string sanitizeURI(const std::string& s);
};
