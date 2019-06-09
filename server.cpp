#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

static void print_time_from(
    const std::chrono::time_point<std::chrono::system_clock> &start_time) {
  auto end_time = std::chrono::high_resolution_clock::now();
  std::cout << "> job took: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                     start_time)
                   .count()
            << " ms" << std::endl;
}

class Session {
public:
  Session(tcp::socket peer)
      : peer_(std::move(peer)), port_(peer_.remote_endpoint().port()),
        address_(peer_.remote_endpoint().address()) {}
  void start();

private:
  // init
  void get_command();
  void get_files();
  void get_filename();
  std::string create_inFile_path();
  void get_fileContent(std::ofstream &&inFile);
  // work
  void compile();
  void send_object_file();
  // end
  void detect_end_of_build();
  void clean_up();

  tcp::socket peer_;
  int port_;
  boost::asio::ip::address address_;
  std::string cmd_;
  std::string inFileName_;
  std::string inFilePath_;

  std::chrono::time_point<std::chrono::system_clock> start_time_ =
      std::chrono::high_resolution_clock::now();
  const std::string receiveFolder_ = "d:/GIT/isqry/test/received/";
};

void Session::start() {
  // protocol:
  // get : command
  // send: acknowledge
  // get : filename
  // send: acknowledge
  // get : file content
  try {
    std::cout << "connected to :" << address_ << ":" << port_ << std::endl;
    get_command();
    get_files();
    print_time_from(start_time_);
  } catch (std::exception &e) {
    std::cerr << "> Session exception: " << e.what() << std::endl;
  }

  std::cout << "disconnected: " << peer_.remote_endpoint().address() << ":"
            << peer_.remote_endpoint().port() << std::endl;
}

void Session::get_command() {
  char buff[32656];
  int bytes_read = 0;
  boost::system::error_code ec;

  for (;;) {
    bytes_read += peer_.read_some(boost::asio::buffer(buff), ec);
    if (ec) {
      throw boost::system::system_error(ec);
    }
    cmd_ += std::string(buff, bytes_read);
    if (cmd_.find("\n") != std::string::npos) {
      cmd_.pop_back();
      std::cout << "> Got cmd: " << cmd_ << std::endl;
      break;
    }
  }
  peer_.wait(boost::asio::socket_base::wait_write);
  peer_.write_some(boost::asio::buffer("OK"), ec);
}

void Session::get_files() {
  std::vector<std::string> fileList(64);

  get_filename();
  get_fileContent(std::ofstream{create_inFile_path()});
}

void Session::get_filename() {
  boost::system::error_code ec;
  int bytes_read = 0;

  for (;;) {
    char buff[32656];
    bytes_read = peer_.read_some(boost::asio::buffer(buff), ec);
    inFileName_ += std::string(buff, bytes_read);

    if (inFileName_.back() == '\n') {
      inFileName_.pop_back();
      std::cout << "> Got fileName: " << inFileName_ << std::endl;
      break;
    } else if (ec) {
      std::cout << "get_filename: error";
      throw boost::system::system_error(ec);
    }
  }
  peer_.wait(boost::asio::socket_base::wait_write);
  peer_.write_some(boost::asio::buffer("OK"), ec);
}

std::string Session::create_inFile_path() {
  fs::path ifp(receiveFolder_ + inFileName_);
  auto ifpp = ifp.parent_path();
  fs::create_directories(ifpp);

  std::cout << "> Create inFilePath: " << ifp.string() << std::endl;
  return ifp.string();
}

void Session::get_fileContent(std::ofstream &&inFile) {
  int bytes_read = 0;
  boost::system::error_code ec;
  if (!inFile.is_open()) {
    throw std::runtime_error("get_fileContent: cannot open file " +
                             inFilePath_);
  }
  for (;;) {
    char buff[65536]{};
    bytes_read = peer_.read_some(boost::asio::buffer(buff), ec);

    if (ec == boost::asio::error::eof) {
      inFile.write(buff, bytes_read);
      inFile.close();
      break; // read all data
    } else if (ec) {
      std::cout << "get_fileContent: error";
      inFile.close();
      throw boost::system::system_error(ec);
    } else {
      inFile.write(buff, bytes_read);
    }
  }
}

class Server {
public:
  Server(boost::asio::io_context &io, short port)
      : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
    do_accept();
  }

private:
  void do_accept() {
    std::cout << "waiting.." << std::endl;
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket))->start();
          }

          this->do_accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main() {
  try {
    constexpr unsigned int port = 3632;
    boost::asio::io_context io;
    Server s(io, port);
    io.run();
  } catch (std::exception &e) {
    std::cerr << "> Server exception: " << e.what() << std::endl;
  }
  return 0;
}
