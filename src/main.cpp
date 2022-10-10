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

        Client client(context, "api.ipify.org", 443, Client::Request::Encryption::TLS);
        client.send(boost::beast::http::verb::get, "?format=json", [](boost::beast::http::status status, const nlohmann::json& json) {
            Log::info(fmt::format("Requst finished ({}): {}", boost::beast::http::obsolete_reason(status).to_string(), json.dump()));
        });

        context.run();
    } catch (const std::exception& ex) {
        Log::warning(fmt::format("Failed: {}", ex.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}