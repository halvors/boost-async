#pragma once

#include <deque>

#include <boost/asio/ip/resolver_base.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include "boost/beast/http.hpp"
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "log.h"

using namespace std::literals::chrono_literals;

typedef std::function<void(boost::beast::http::status, const nlohmann::json&)> HttpResponseHandler;

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

    virtual void asyncHandshake() { processQueue(); }
    virtual void asyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req) = 0;
    virtual void asyncRead() = 0;

    void handleWrite(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/)
    {
        if (error)
            return handleError(error);

        asyncRead();
    }
    
    void handleRead(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/)
    {
        if (error)
            return handleError(error);

        // Parse body
        //std::string body = boost::beast::buffers_to_string(res.body().data());
        std::string& body = res.body();
        nlohmann::json json;

        if (nlohmann::json::accept(body))
            json = nlohmann::json::parse(body);
        
        // Call handler function
        auto handler = queue.front().second;
        handler(res.result(), json);

        // Remove request from queue
        queue.pop_front();

        // Cleanup
        res.clear();
        body.clear();

        // Continue to process queue
        processQueue();
    }
    
    // virtual void shutdown(boost::asio::ip::tcp::socket::shutdown_type type, boost::beast::error_code& error)
    // {
    //     getStream().socket().shutdown(type, error);
    // }

    void enqueue(boost::beast::http::verb method, const std::string& target, HttpResponseHandler handler, const nlohmann::json& json)
    {
        // Set up an HTTP GET request message
        boost::beast::http::request<boost::beast::http::string_body> req;
        req.set(boost::beast::http::field::host, query.host_name());
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        req.method(method);

        if (method == boost::beast::http::verb::post) {
            req.set(boost::beast::http::field::content_type, "application/json");
            req.body() = json.dump();
            req.prepare_payload();
        }

        req.version(11);
        req.target(target);

        // TODO: Handle json body for POST etc?

        // Queue request for processing later
        queue.emplace_back(std::move(req), handler);

        // Start processing of queue if no other pending
        if (queue.size() == 1)
            processQueue();
    }

    void processQueue()
    {
        // Return in case of empty queue
        if (queue.empty())
            return;

        if (!running) {
            // Look up the domain name
            return resolver.async_resolve(query, [this](const boost::beast::error_code error, boost::asio::ip::tcp::resolver::results_type results) {
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
                    [this](const boost::beast::error_code error, const boost::asio::ip::tcp::resolver::results_type::endpoint_type& endpoint) {
                        if (error)
                            return handleError(error);

                        Log::info(fmt::format("Connected to {}:{}", endpoint.address().to_string(), endpoint.port())); 

                        // Mark connection successful
                        running = true;

                        // Perform handshake
                        asyncHandshake();
                    });
            });
        }
            
        asyncWrite(queue.front().first);
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
    //boost::beast::http::response<boost::beast::http::dynamic_body> res;
    boost::beast::http::response<boost::beast::http::string_body> res;

    static constexpr std::chrono::seconds timeout = 30s;
};
