#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {

    const char *port = "3632";            // the port we connect
    const char *server = "10.76.153.34";
    // const unsigned int buff_size = 65536; // the size of the read buffer

    std::cout << "Usage: type text and press enter to send." << std::endl;

    boost::asio::io_service io_service;        // asio main object
    tcp::resolver resolver(io_service);        // a resolver for name to @
    tcp::resolver::query (server, port); // ask the dns for this resolver
    tcp::resolver::iterator endpoint = resolver.resolve(tcp::resolver::query (server, port)); // iterator if multiple answers for a given name
    tcp::resolver::iterator end;

    tcp::socket socket(io_service); // attach a socket to the main asio object
    socket.connect(*endpoint); // connect to the first returned object
    // boost::system::error_code error;
    // unsigned int count = 0;              // a counter
    while (1) {         // loop until there is no more data to send
      std::string msg = "";
      std::cin >> msg;
      size_t len = socket.send(boost::asio::buffer(msg));
      std::cout << "Sent '" << msg << "'("<< len << "bytes)" << std::endl;

      msg = ""; // create read buffer
      len = socket.receive(boost::asio::buffer(msg)); // read data
      std::cout << "Got '" << msg << "' (" << len << "bytes)" << std::endl;
    }
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
