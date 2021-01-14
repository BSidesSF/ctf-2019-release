#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace opendoor {

  class Buffer {
    public:
      // Constructors
      Buffer();
      Buffer(const Buffer &);
      Buffer(std::shared_ptr<std::vector<std::uint8_t>>);
      ~Buffer();

      // Allow reading basic types from buffer
      template <typename T>
      T read();

      // Allow writing objects
      template <typename T>
      uint32_t write(T val);

      // Get contents of underlying buffer.
      std::shared_ptr<std::vector<std::uint8_t>> get_contents();

      // Print for debugging
      std::string debug_string();
      int write_debug(int fd);
      int write_debug();

    private:
      std::shared_ptr<std::vector<std::uint8_t>> data_buf_;

      uint32_t offset_;
  }; // class Buffer

} // namespace opendoor

#endif
