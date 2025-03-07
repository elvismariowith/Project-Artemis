#include "client.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


std::string client::getImage()
{
    auto const host = "192.168.245.13";
    auto const port = "80";
    auto const target = "/stream";
    int version = 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::connection, "keep_alive");
    http::write(stream, req);
    http::chunk_extensions ce;
    beast::flat_buffer buffer;
    http::response_parser<http::empty_body> parser;
    beast::error_code ec;
    std::string chunk;

    auto header_cb =
        [&chunk,&ce](std::uint64_t size,          // Size of the chunk, or zero for the last chunk
            std::string_view extensions, // The raw chunk-extensions string. Already validated.
            beast::error_code &ev)       // We can set this to indicate an error
    {
        // Parse the chunk extensions so we can access them easily
        ce.parse(extensions, ev);
        if (ev)
            return;

        // See if the chunk is too big
        if (size > (std::numeric_limits<std::size_t>::max)())
        {
            ev = http::error::body_limit;
            return;
        }

        // Make sure we have enough storage, and
        // reset the container for the upcoming chunk
        chunk.reserve(static_cast<std::size_t>(size));
        chunk.clear();
    };
    auto body_cb =
        [&chunk](std::uint64_t remain,  // The number of bytes left in this chunk
            std::string_view body, // A buffer holding chunk body data
            beast::error_code &ec) // We can set this to indicate an error
    {
        // If this is the last piece of the chunk body,
        // set the error so that the call to `read` returns
        // and we can process the chunk.
        if (remain == body.size())
            ec = http::error::end_of_chunk;

        // Append this piece to our container
        chunk.append(body.data(), body.size());

        // The return value informs the parser of how much of the body we
        // consumed. We will indicate that we consumed everything passed in.
        return body.size();
    };
    parser.on_chunk_header(header_cb);
    parser.on_chunk_body(body_cb);
    for (int i = 0; i < 3; i++)
    {
        // Read as much as we can. When we reach the end of the chunk, the chunk
        // body callback will make the read return with the end_of_chunk error.
        http::read(stream, buffer, parser, ec);
        if (!ec)
        {
            continue;
        }
        else if (ec != http::error::end_of_chunk)
        {
            std::cout <<"chunk not ended" << '\n';
            return 0;
        }
        else
            ec.assign(0, ec.category());

        // We got a whole chunk, print the extensions:
        for (auto const &extension : ce)
        {
            std::cout << "Extension: " << extension.first;
            if (!extension.second.empty())
                std::cout << " = " << extension.second << std::endl;
            else
                std::cout << std::endl;
        }

        // Now print the chunk body
    }
    return chunk;
}