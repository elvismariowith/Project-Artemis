#ifndef CLIENT
#define CLIENT

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/chunk_encode.hpp>
#include <string>



namespace client
{
    std::string getImage();
}
#endif