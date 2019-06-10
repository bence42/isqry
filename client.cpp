#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

// protocol:
// send : command
// send : filename
// send : file content

int main() {
  try {
    const char* port = "3632";
    const char* server = "192.168.1.107";

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query(server, port);
    tcp::resolver::iterator endpoint =
        resolver.resolve(tcp::resolver::query(server, port));

    tcp::socket socket(io_service);
    socket.connect(*endpoint);

    // send command
    std::string cmd = "gcc -c data.cpp -o data.exe -I./sent/include";
    cmd += "\n";
    size_t request_length = cmd.size();
    auto bytes_sent = boost::asio::write(
        socket, boost::asio::buffer(cmd.c_str(), request_length));
    cmd.pop_back();
    std::cout << "> Sent cmd: '" << cmd << "' " << std::endl;

    // open file
    std::string fileName = "test/to_send/data.cpp";
    std::ifstream file(fileName, std::ios_base::in);
    if (!file.is_open()) {
      std::cerr << "> error: cant open file: " << fileName << std::endl;
      socket.close();
      exit(1);
    }

    // send filename
    fileName += "\n";
    request_length = fileName.size();
    bytes_sent = boost::asio::write(
        socket, boost::asio::buffer(fileName, request_length));
    fileName.pop_back();

    std::cout << "> Sent filename: '" << fileName << "' " << std::endl;

    // send file content

    while (!file.eof()) {
      constexpr auto buff_size = 65536;
      char buff[buff_size]{};

      file.read(buff, buff_size);
      size_t request_length = file.gcount();
      auto bytes_sent =
          boost::asio::write(socket, boost::asio::buffer(buff, request_length));
    }
    file.close();
  } catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
