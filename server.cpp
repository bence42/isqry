#include <boost/asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(tcp::socket&& peer)
      : peer_(std::move(peer)),
        port_(peer_.remote_endpoint().port()),
        address_(peer_.remote_endpoint().address()),
        start_time_(std::chrono::high_resolution_clock::now()) {
    report("> connected to :", address_, ":", port_);
  }

  Session(const Session&) = delete;
  Session(const Session&&) = delete;

  ~Session() {
    report_end();
    if (inFile_.is_open()) {
      inFile_.close();
    }
  }

  void start();

 private:
  // utils
  template <typename... Args>
  inline void report(Args&&... args);
  void print_time_from(
      const std::chrono::time_point<std::chrono::system_clock>& start_time);
  // init
  void get_command();
  void get_filename();
  std::string create_inFile_path();
  void prepare_file();
  void get_fileContent();
  // work
  void compile();
  void send_object_file();
  // end
  void detect_end_of_build();
  void clean_up();
  void report_end();

  tcp::socket peer_;
  const int port_;
  const boost::asio::ip::address address_;

  std::string cmd_;
  std::string inFileName_;
  std::string inFilePath_;
  std::ofstream inFile_;

  static constexpr int max_buff_length = 65536;
  char fileChunk_[max_buff_length]{};
  boost::asio::streambuf sbuff_;

  const std::chrono::time_point<std::chrono::system_clock> start_time_;
  const std::string receiveFolder_ = "d:/GIT/isqry/test/received/";
};

void Session::start() {
  // protocol:
  // get : command
  // get : filename
  // get : file content
  try {
    get_command();
  } catch (std::exception& e) {
    report("> error: Session init exception : ", e.what());
  }
}

template <typename... Args>
inline void Session::report(Args&&... args) {
  (std::cout << ... << args);
  std::cout << "\n";
}

void Session::print_time_from(
    const std::chrono::time_point<std::chrono::system_clock>& start_time) {
  auto end_time = std::chrono::high_resolution_clock::now();
  report("> job took: ",
         std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                               start_time)
             .count(),
         " ms");
}

void Session::get_command() {
  auto lifetime_mngr = shared_from_this();
  boost::asio::async_read_until(
      peer_, sbuff_, "\n",
      [this, lifetime_mngr](const boost::system::error_code error,
                            std::size_t bytes_transferred) {
        if (!error) {
          cmd_ = std::string{buffers_begin(sbuff_.data()),
                             buffers_begin(sbuff_.data()) + bytes_transferred -
                                 std::string{"\n"}.size()};
          sbuff_.consume(bytes_transferred);
          report("> got cmd : '", cmd_, "'");
          get_filename();
        } else {
          report("> error: get_command failed to receive cmd");
          throw boost::system::system_error(error);
        }
      });
}

void Session::get_filename() {
  auto lifetime_mngr = shared_from_this();
  boost::asio::async_read_until(
      peer_, sbuff_, "\n",
      [this, lifetime_mngr](const boost::system::error_code error,
                            std::size_t bytes_transferred) {
        if (!error) {
          inFileName_ =
              std::string{buffers_begin(sbuff_.data()),
                          buffers_begin(sbuff_.data()) + bytes_transferred -
                              std::string{"\n"}.size()};
          sbuff_.consume(bytes_transferred);
          report("> got inFileName : '", inFileName_, "'");
          prepare_file();
        } else {
          report("> error: get_filename failed to receive filename");
          throw boost::system::system_error(error);
        }
      });
}

std::string Session::create_inFile_path() {
  fs::path ifp(receiveFolder_ + inFileName_);
  auto ifpp = ifp.parent_path();
  fs::create_directories(ifpp);

  inFilePath_ = ifp.string();
  report("> Create inFilePath : '", inFilePath_, "'");
  return inFilePath_;
}

void Session::prepare_file() {
  inFile_.open(create_inFile_path());
  if (inFile_.fail()) {
    report("> error: prepare_file open failure : ", strerror(errno));
    report("> note : prepare_file tired to open :", inFilePath_);
  }
  if (!inFile_.is_open()) {
    throw std::runtime_error("prepare_file cannot open file :" + inFilePath_);
  }

  get_fileContent();
}

void Session::get_fileContent() {
  auto lifetime_mngr = shared_from_this();
  peer_.async_read_some(
      boost::asio::buffer(fileChunk_),
      [this, lifetime_mngr](const boost::system::error_code error,
                            std::size_t bytes_transferred) {
        if (error == boost::asio::error::eof) {
          report("> get_fileContent: done");
          inFile_.write(static_cast<char*>(fileChunk_), bytes_transferred);
        } else if (error) {
          report("> error: get_fileContent: error during receiving file");
          throw boost::system::system_error(error);
        } else {
          inFile_.write(static_cast<char*>(fileChunk_), bytes_transferred);
          memset(static_cast<char*>(fileChunk_), 0, bytes_transferred);
          get_fileContent();
        }
      });
}

void Session::report_end() {
  report("> disconnected :", peer_.remote_endpoint().address(), ":",
         peer_.remote_endpoint().port());
  print_time_from(start_time_);
}

class Server {
 public:
  Server(boost::asio::io_context& io, int16_t port)
      : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
    do_accept();
  }

 private:
  void do_accept() {
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
  } catch (std::exception& e) {
    std::cout << "> error: Server exception : " << e.what() << std::endl;
  }
  return 0;
}
