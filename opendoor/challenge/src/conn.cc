#include "common.h"
#include "conn.h"
#include "message.h"

#include <algorithm>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace opendoor {

  ConnectionHandler::ConnectionHandler(int fd)
    : fd_(fd),
      closed_(false),
      msglen_(-1),
      outbuf_(),
      inbuf_(),
      pending_buf_(),
      state_(std::make_shared<State>()),
      crypter_(std::shared_ptr<Crypter>(
            dynamic_cast<Crypter *>(new PlainCrypter())))
  {
    DBG("Handling connection %d", fd);
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) {
      perror("fcntl");
      fail();
      return;
    }
    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
      perror("fcntl");
      fail();
    }
  }

  ConnectionHandler::~ConnectionHandler() {
    if (fd_ != -1)
      close(fd_);
    pending_buf_.reset();
  }

  bool ConnectionHandler::closed() {
    return closed_;
  }

  void ConnectionHandler::fail() {
    DBG("Connection %d failed or closed.", fd_);
    inbuf_.clear();
    msglen_ = -1;
    if (fd_ != -1)
      close(fd_);
    closed_ = true;
  }

  bool ConnectionHandler::do_work(bool readable, bool writable) {
    bool ok = true;
    try {
      if (readable) {
        ok &= do_read_();
      }
      if (!pending_write()) {
        // Check if we should start a new write.
        auto buf = read_buffer_();
        if (buf) {
          DBG("Processing message for connection %d", fd_);
          auto resp = process_message_(buf);
          if (resp) {
            write_buffer_(resp);
          }
        }
      }
      if (writable && pending_write()) {
        ok &= do_write_();
      }
    } catch(...) {
      fail();
      return false;
    }
    return ok;
  }

  bool ConnectionHandler::pending_write() {
    return outbuf_.size() > 0;
  }

  void ConnectionHandler::install_crypter(std::shared_ptr<Crypter> crypter) {
    crypter_ = crypter;
  }

  std::shared_ptr<Buffer> ConnectionHandler::process_message_(
      std::shared_ptr<Buffer> input) {
    auto plaintext = crypter_->decrypt(input);
    if (!plaintext) {
      DBG("Decryption failed.");
      return crypter_->encrypt(
          (new ErrorMessage(ErrorMessage::Err_Crypto))->serialize());
    }
#ifdef DEBUG
    DBG("Plaintext: ");
    plaintext->write_debug();
#endif
    auto msg = Message::ParseMessage(plaintext);
    if (!msg) {
      DBG("Parsing failed.");
      return crypter_->encrypt(
          (new ErrorMessage(ErrorMessage::Err_Parsing))->serialize());
    }
    auto resp = msg->execute(state_);
    if (!resp) {
      DBG("Execution failed.");
      return crypter_->encrypt(
          (new ErrorMessage(ErrorMessage::Err_Unknown))->serialize());
    }
    return crypter_->encrypt(resp->serialize());
  }

  bool ConnectionHandler::do_read_() {
    ssize_t r;
    int32_t val;
    if (msglen_ == -1) {
      inbuf_.clear();
      r = read(fd_, &val, sizeof(val));
      if (r != sizeof(msglen_)) {
        fail();
        return false;
      }
      msglen_ = ntohl(val);
      DBG("Expecting message length: %d", msglen_);
      if (msglen_ < 1) {
        DBG("Message length too short: %d", msglen_);
        fail();
        return false;
      }
      inbuf_.reserve(msglen_);
    }
    size_t sz = inbuf_.size();
    DBG("Current buf size: %lu", sz);
    uint8_t *buf = inbuf_.data();
    inbuf_.resize(msglen_);
    r = read(fd_, buf + sz, msglen_ - sz);
    if (r == -1) {
#ifdef DEBUG
      perror("read");
#endif
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        return true;
      fail();
      return false;
    } else if (r == 0) {
      // 0 read when "ready" == closed
      fail();
      return false;
    }
    inbuf_.resize(sz + r);
    return true;
  }

  std::shared_ptr<Buffer> ConnectionHandler::read_buffer_() {
    if (msglen_ == -1 || msglen_ != (ssize_t) inbuf_.size()) {
      if (msglen_ > 0)
        DBG("Expected length %d, have %lu.", msglen_, inbuf_.size());
      return std::shared_ptr<Buffer>(nullptr);
    }
    std::shared_ptr<Buffer> buf = std::shared_ptr<Buffer>(new Buffer());
    buf->write(&inbuf_);
    inbuf_.clear();
    msglen_ = -1;
    return buf;
  }

  void ConnectionHandler::write_buffer_(std::shared_ptr<Buffer> buf) {
    outbuf_.insert(
        outbuf_.end(),
        buf->get_contents()->begin(),
        buf->get_contents()->end());
  }

  bool ConnectionHandler::do_write_() {
    if (!pending_write())
      return false;
    ssize_t r = write(fd_, outbuf_.data(), outbuf_.size());
    if (r == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        return true;
      fail();
      return false;
    }
    // optimize for common case
    if (r == (ssize_t) outbuf_.size()) {
      outbuf_.clear();
      return true;
    }
    outbuf_.erase(outbuf_.begin(), outbuf_.begin() + r);
    return true;
  }

  ConnectionPool::ConnectionPool()
    : port_(DEFAULT_PORT),
      sock_fd_(-1),
      connmap_(),
      crypter_(std::shared_ptr<Crypter>(
            dynamic_cast<Crypter *>(new PlainCrypter())))
  {}

  ConnectionPool::~ConnectionPool() {
    crypter_.reset((Crypter *)nullptr);
  }

  void ConnectionPool::set_port(uint16_t port) {
    port_ = port;
  }

  int ConnectionPool::listen() {
    connmap_.clear();

    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_ == -1) {
      perror("socket");
      return -1;
    }

    int enable = 1;
    if (setsockopt(
          sock_fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
      perror("setsockopt");
      return -1;
    }

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(struct sockaddr_in));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port_);
    if (bind(
          sock_fd_, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1) {
      perror("bind");
      return -1;
    }

    if (::listen(sock_fd_, 10) == -1) {
      perror("listen");
      return -1;
    }

    return 0;
  }

  int ConnectionPool::iterate() {
    int nfds = 0;
    fd_set rfds;
    fd_set wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    nfds = sock_fd_ + 1;
    FD_SET(sock_fd_, &rfds);

    auto itr = connmap_.begin();
    while (itr != connmap_.end()) {
      if (itr->second->closed()) {
        itr = connmap_.erase(itr);
        continue;
      }
      if (itr->second->pending_write()) {
        FD_SET(itr->first, &wfds);
      }
      FD_SET(itr->first, &rfds);
      nfds = std::max(nfds, itr->first + 1);
      ++itr;
    }

    if (select(nfds, &rfds, &wfds, NULL, NULL) == -1) {
      perror("select");
      return -1;
    }

    if (FD_ISSET(sock_fd_, &rfds)) {
      int client;
      if ((client = accept(sock_fd_, NULL, NULL)) == -1) {
        perror("accept");
        return -1;
      }
      handle_conn_(client);
    }

    for (int i=0; i<nfds; i++) {
      if (i == sock_fd_)
        continue;
      bool readable = FD_ISSET(i, &rfds);
      bool writable = FD_ISSET(i, &wfds);
      if (readable || writable)
        connmap_.at(i)->do_work(readable, writable);
    }

    return 0;
  }

  void ConnectionPool::install_crypter(std::shared_ptr<Crypter> crypter) {
    crypter_ = crypter;
  }

  bool ConnectionPool::handle_conn_(int fd) {
    ConnectionHandler *conn = new ConnectionHandler(fd);
    connmap_[fd] = std::shared_ptr<ConnectionHandler>(conn);
    conn->install_crypter(crypter_);
    return true;
  }

} // namespace opendoor
