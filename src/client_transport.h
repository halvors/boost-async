#pragma once

#include <deque>

#include <boost/asio/ip/resolver_base.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include "boost/beast/http.hpp"
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <fmt/format.h>

#include "log.h"

using namespace std::literals::chrono_literals;

typedef std::function<void(boost::beast::http::status)> HttpResponseHandler;

class ClientTransport
{
public:
    explicit ClientTransport(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port) : 
        query(host, std::to_string(port)),
        resolver(ioContext)
    {
        
    }

    virtual ~ClientTransport() = default;

    virtual boost::beast::tcp_stream& getStream() = 0;
    virtual boost::beast::error_code setHostname([[maybe_unused]] const std::string& hostname) { return {}; }

    virtual void async_handshake() { processQueue(); }

    virtual void write(boost::beast::http::request<boost::beast::http::string_body>& req, HttpResponseHandler handler) = 0;

    // virtual void shutdown(boost::asio::ip::tcp::socket::shutdown_type type, boost::beast::error_code& error)
    // {
    //     getStream().socket().shutdown(type, error);
    // }

    void queueRequest(boost::beast::http::verb method, const std::string& target, HttpResponseHandler handler)
    {
        // Set up an HTTP GET request message
        boost::beast::http::request<boost::beast::http::string_body> beastReq;
        beastReq.set(boost::beast::http::field::host, query.host_name());
        beastReq.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        beastReq.method(method);
        //beastReq.version(11);
        beastReq.target(target);

        Log::info("Queueing request for processing..."); 

        // Queue request for processing later
        queue.emplace_back(std::move(beastReq), handler);

        if (queue.size() == 1) // no pending
            processQueue();
    }

    void processQueue()
    {
        // If queue is empty we cannot process the queue
        if (queue.empty())
            return;

        if (!running) {
            // Look up the domain name
            return resolver.async_resolve(query, [this](boost::beast::error_code error, boost::asio::ip::tcp::resolver::results_type results) {
                if (error)
                    return handleError(error);

                // TODO: For debugging print results of resolution
                for (const auto& r : results) {
                    Log::info(fmt::format("{} -> {}:{}", 
                            r.host_name(), 
                            r.endpoint().address().to_string(), 
                            r.endpoint().port()));
                }

                getStream().expires_after(timeout);

                // Make the connection on any of the endpoints discovered by resolve
                getStream().async_connect(results,
                    [this](boost::beast::error_code error, boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint) {
                        if (error)
                            return handleError(error);

                        Log::info(fmt::format("Connected to {}:{}", endpoint.address().to_string(), endpoint.port())); 

                        // Mark connection successful
                        running = true;

                        // Perform handshake
                        async_handshake();
                    });
            });
        } else {
            auto req = queue.front().first;

            write(req, queue.front().second);

            // request done
            queue.pop_front();
        }

        //getStream().expires_after(timeout);

        // boost::beast::http::async_write(getStream(), req,
        //     std::bind(&ClientTransport::async_write, this, std::placeholders::_1));

        // boost::beast::http::async_write(getStream(), req,
        //     [](boost::beast::error_code error, const size_t) {
        //         // if (error)
        //         //     return handleError(error);

        //         Log::error("Write works"); 
        //     });
    }
    
    void handleError(boost::system::error_code error)
    {
        if (!error)
            return;

        Log::warning(fmt::format("Error: {} ({})", error.message(), error.value()));
    }

protected:
    boost::asio::ip::tcp::resolver::query query;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::flat_buffer buffer;

    std::deque<std::pair<boost::beast::http::request<boost::beast::http::string_body>, HttpResponseHandler>> queue;
    bool running = false;

    static constexpr std::chrono::seconds timeout = 80s;
};
