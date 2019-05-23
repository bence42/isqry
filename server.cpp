#include <boost/asio.hpp> 
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
  try {
    const unsigned int port = 3632;       // port the server listen
    // const unsigned int buff_size = 16384; // size of the send buffer

    boost::asio::io_service io_service;           // main asio object
    tcp::endpoint endpoint(tcp::v4(), port);      // endpoint
    tcp::acceptor acceptor(io_service, endpoint); // we accept connection there

    // server loop
    while (1) {
      tcp::socket socket(io_service); // create a socket
      acceptor.accept(socket);        // attach to the acceptor
      // we have got a new connection !
      std::cout << " Remote @:port  " << socket.remote_endpoint().address()
                << " : " << socket.remote_endpoint().port() << std::endl;
      socket.wait(boost::asio::socket_base::wait_read);
      std::string msg = ""; // create read buffer
      size_t len = socket.receive(boost::asio::buffer(msg)); // read data
      std::cout << "Got '" << msg << "' (" << len << "bytes)" << std::endl;

      len = socket.send(boost::asio::buffer("OK"));
      std::cout << "Sent OK (" << len << " bytes)" << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
  