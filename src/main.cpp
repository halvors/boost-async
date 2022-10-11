#include <exception>

#include <boost/asio/io_context.hpp>
#include <fmt/format.h>

#include "client.h"
#include "log.h"

using namespace boost;

int main()
{
    try {
        asio::io_context context;
        // auto workGuard = asio::make_work_guard(context);

        Client client(context, "api.ipify.org", 80, Client::Request::Encryption::NONE);
        client.send(boost::beast::http::verb::get, "?format=json", [&client](boost::beast::http::status status, const nlohmann::json& json) {
            Log::info(fmt::format("Requst finished ({}): {}", boost::beast::http::obsolete_reason(status).to_string(), json.dump()));
        
            // boost::beast::error_code error;
            // client.transport->shutdown(boost::asio::ip::tcp::socket::shutdown_type::shutdown_both, error);

            // if (error)
            //     Log::error(fmt::format("Error: {} ({})", error.message(), error.value()));
            // else
            //     Log::info("A ok!");
        });

        context.run();
    } catch (const std::exception& ex) {
        Log::warning(fmt::format("Failed: {}", ex.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}