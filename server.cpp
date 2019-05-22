#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>


using boost::asio::ip::tcp;

int main() {
  try {
    const unsigned int port = 3632;       // port the server listen
    const unsigned int buff_size = 16384; // size of the send buffer

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

      std::fstream file("client.cpp"); // we open this file

      char *buff = new char[buff_size]; // creating the buffer
      unsigned int count = 0;           // counter
      std::cout << "Sending" << std::endl;
      while (!file.eof()) {         // loop until there is no more data to send
        memset(buff, 0, buff_size); // cleanup the buffer
        file.read(buff, buff_size); // read some data
        boost::system::error_code ignored_error;
        unsigned int len =
            file.gcount(); // get the effective number of bytes read
        boost::asio::write(socket, boost::asio::buffer(buff, len),
                           boost::asio::transfer_all(), ignored_error); // send
        count += len; // increment counter
      }

      file.close();  // close file
      delete[] buff; // delete buffer
      std::cout << "Finished" << std::endl;
      std::cout << "Sent " << count << " bytes" << std::endl;
    }

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
