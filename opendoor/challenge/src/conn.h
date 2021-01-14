#ifndef _CONN_H_
#define _CONN_H_

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "crypter.h"
#include "buffer.h"
#include "state.h"

#define DEFAULT_PORT 4848

namespace opendoor {

  class ConnectionHandler {
    public:
      ConnectionHandler(int fd);
      ~ConnectionHandler();
      virtual bool closed();
      virtual void fail();
      virtual bool pending_write();
      virtual bool do_work(bool readable, bool writable);
      virtual void install_crypter(std::shared_ptr<Crypter>);

    private:
      virtual std::shared_ptr<Buffer> read_buffer_();
      virtual std::shared_ptr<Buffer> process_message_(std::shared_ptr<Buffer>);
      virtual void write_buffer_(std::shared_ptr<Buffer>);
      virtual bool do_read_();
      virtual bool do_write_();

      int fd_;
      bool closed_;
      int32_t msglen_;
      std::vector<uint8_t> outbuf_;
      std::vector<uint8_t> inbuf_;
      std::shared_ptr<Buffer> pending_buf_;
      std::shared_ptr<State> state_;
      std::shared_ptr<Crypter> crypter_;
  }; // class ConnectionHandler

  class ConnectionPool {
    public:
      ConnectionPool();
      ~ConnectionPool();
      void set_port(uint16_t port);
      virtual int listen();
      virtual int iterate();
      virtual void install_crypter(std::shared_ptr<Crypter>);

    private:
      virtual bool handle_conn_(int fd);
      uint16_t port_;
      int sock_fd_;
      std::map<int, std::shared_ptr<ConnectionHandler>> connmap_;
      std::shared_ptr<Crypter> crypter_;

  }; // class ConnectionPool

} // namespace opendoor

#endif
