#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class Server {
public:
  Server(const unsigned short port) : port_(port) {}

  void start();
  void accept_job(tcp::socket);
  std::string get_command(tcp::socket &);
  void get_files();
  void compile();
  void send_object_file();
  void detect_end_of_build();
  void clean_up();

private:
  boost::asio::io_context io_;
  unsigned short port_;
};

void Server::start() {
  tcp::acceptor a(io_, tcp::endpoint(tcp::v4(), port_));
  for (;;) {
    accept_job(a.accept());
  }
}

void Server::accept_job(tcp::socket socket) {
  try {
    std::cout << "connected to client:" << std::endl;
    std::cout << socket.remote_endpoint().address() << ":"
              << socket.remote_endpoint().port() << std::endl;

    std::cout << get_command(socket) << std::endl;

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  std::cout << "disconnected: " << socket.remote_endpoint().address()
            << std::endl;
}

std::string Server::get_command(tcp::socket &socket) {
  char msg[32656];
  int msg_len = 0;
  boost::system::error_code ec;
  for (;;) {
    msg_len += socket.read_some(boost::asio::buffer(msg), ec);
    if (ec == boost::asio::error::eof) {
      break; // read all data
    } else if (ec) {
      throw boost::system::system_error(ec);
    }
  }

  return std::string(msg, msg_len);
}

int main() {
  try {
    constexpr unsigned int port = 3632;
    Server s(port);
    s.start();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
