#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {

    const char *port = "3632";
    const char *server = "10.76.153.34";

    std::cout << "Usage: type text and press enter to send." << std::endl;

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query(server, port);
    tcp::resolver::iterator endpoint =
        resolver.resolve(tcp::resolver::query(server, port));
    tcp::resolver::iterator end;

    for (;;) {
      tcp::socket socket(io_service);
      socket.connect(*endpoint);
      auto max_length = 32656;
      char msg[max_length];
      std::cin.getline(msg, max_length);
      size_t request_length = std::strlen(msg);
      auto len = boost::asio::write(socket, boost::asio::buffer(msg, request_length));
      std::cout << "Sent: '" << msg << "' " << len << std::endl;

    }
  } catch (std::exception &e) { 
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
