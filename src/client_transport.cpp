#include "client_transport.h"


#include <boost/beast/core/buffers_to_string.hpp>
#include "boost/beast/http.hpp"
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <fmt/format.h>

#include "log.h"

ClientTransport::ClientTransport(boost::asio::io_context& ioContext, const std::string& host, std::uint16_t port) : 
    query(host, std::to_string(port)),
    resolver(ioContext)
{
    
}

void ClientTransport::enqueue(boost::beast::http::verb method, const std::string& target, HttpResponseHandler handler, const nlohmann::json& json)
{
    // Set up an HTTP GET request message
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.set(boost::beast::http::field::host, query.host_name());
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Only add body for appropriate methods
    if (method == boost::beast::http::verb::post ||
        method == boost::beast::http::verb::put ||
        method == boost::beast::http::verb::patch) {
        req.set(boost::beast::http::field::content_type, "application/json");
        req.body() = json.dump();
        req.prepare_payload();
    }

    req.method(method);
    req.version(11);
    req.target(sanitizeURI(target));

    // Queue request for processing later
    queue.emplace_back(std::move(req), handler);

    // Start processing of queue if no other pending
    if (queue.size() == 1)
        processQueue();
}

void ClientTransport::handleWrite(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/)
{
    if (error)
        return handleError(error);

    asyncRead();
}

void ClientTransport::handleRead(const boost::beast::error_code error, const std::size_t /*bytesTransferred*/)
{
    if (error)
        return handleError(error);

    // Parse body
    std::string body = boost::beast::buffers_to_string(res.body().data());
    nlohmann::json json;

    if (nlohmann::json::accept(body))
        json = nlohmann::json::parse(body);
    
    // Call handler function
    auto handler = queue.front().handler;
    handler(res.result(), json);

    // Remove request from queue
    queue.pop_front();

    // Cleanup
    res.clear();
    res.body().clear();

    // Continue to process queue
    processQueue();
}

void ClientTransport::shutdown(boost::asio::ip::tcp::socket::shutdown_type type, boost::beast::error_code& error)
{
    getStream().socket().shutdown(type, error);
}

void ClientTransport::processQueue()
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

    // Write request
    asyncWrite(queue.front().req);
}

void ClientTransport::handleError(boost::system::error_code error)
{
    if (!error)
        return;

    Log::warning(fmt::format("Error: {} ({})", error.message(), error.value()));
}

std::string ClientTransport::sanitizeURI(const std::string& s)
{
    if (s.front() != '/')
        return '/' + s;

    return s;
}
