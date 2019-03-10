#include "buffer.h"
#include "common.h"

#include <endian.h>
#include <string>
#include <unistd.h>

namespace opendoor {

Buffer::Buffer()
  : data_buf_(std::make_shared<std::vector<std::uint8_t>>()),
    offset_(0) {
      DBG("Construct empty buffer: %p", this);
    }

Buffer::Buffer(const Buffer &buf)
  : data_buf_(std::shared_ptr<std::vector<std::uint8_t>>(buf.data_buf_)),
    offset_(0) {
      DBG("Copy buffer: %p", this);
    }

Buffer::Buffer(std::shared_ptr<std::vector<std::uint8_t>> buf)
  : data_buf_(buf),
    offset_(0) {
      DBG("Buffer from vector: %p", this);
    }

Buffer::~Buffer() {
  DBG("Destroy buffer %p", this);
}

std::shared_ptr<std::vector<std::uint8_t>> Buffer::get_contents() {
  return data_buf_;
}

template <typename T>
T Buffer::read() {
  union {
    T val;
    uint8_t buf[sizeof(T)];
  } tmp;
  for (uint32_t i=0; i<sizeof(T); i++) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    tmp.buf[sizeof(T) - i - 1] = read<uint8_t>();
#else
    tmp.buf[i] = read<uint8_t>();
#endif
  }
  return tmp.val;
}

/* Specialization of read for single bytes. */
template <>
uint8_t Buffer::read() {
  return data_buf_->at(offset_++);
}

/* Specialization of read for strings. */
template <>
std::string Buffer::read() {
  int len = read<uint32_t>();
  auto rv = std::string(data_buf_->begin() + offset_,
      data_buf_->begin() + offset_ + len);
  offset_ += len;
  return rv;
}

/* Specialization of read for vectors. */
template <>
std::shared_ptr<std::vector<uint8_t>> Buffer::read() {
  int len = read<uint32_t>();
  auto rv = new std::vector<uint8_t>(data_buf_->begin() + offset_,
      data_buf_->begin() + offset_ + len);
  offset_ += len;
  return std::shared_ptr<std::vector<uint8_t>>(rv);
}

template <typename T>
uint32_t Buffer::write(T val) {
  union {
    T val;
    uint8_t buf[sizeof(T)];
  } tmp;
  tmp.val = val;
  for (uint32_t i=0; i<sizeof(T); i++) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    data_buf_->push_back(tmp.buf[sizeof(T) - i - 1]);
#else
    data_buf_->push_back(tmp.buf[i]);
#endif
  }
  return sizeof(T);
}

template <>
uint32_t Buffer::write(uint8_t val) {
  data_buf_->push_back(val);
  return 1;
}

template <>
uint32_t Buffer::write(std::string *val) {
  write<uint32_t>(val->size());
  for (uint32_t i=0; i<val->size(); i++) {
    write((char)(*val)[i]);
  }
  return val->size() + sizeof(uint32_t);
}

template <>
uint32_t Buffer::write(std::shared_ptr<std::vector<std::uint8_t>> val) {
  write<uint32_t>(val->size());
  for (uint32_t i=0; i<val->size(); i++) {
    write((*val)[i]);
  }
  return val->size() + sizeof(uint32_t);
}

template <>
uint32_t Buffer::write(std::vector<std::uint8_t> *val) {
  write<uint32_t>(val->size());
  for (uint32_t i=0; i<val->size(); i++) {
    write((*val)[i]);
  }
  return val->size() + sizeof(uint32_t);
}

std::string Buffer::debug_string() {
  std::string s;
  char tmp[4];
  s.reserve(data_buf_->size() * 4); // a little extra
  for (uint32_t i=0; i<data_buf_->size(); i++) {
    if (i % 8 == 0) {
      // beginning, mid of line
      s += " ";
    }
    sprintf(tmp, " %02x", (*data_buf_)[i]);
    s += tmp;
    if (i % 16 == 15) {
      s += "\n";
    }
  }
  if (s[s.length() - 1] != '\n')
    s += "\n";
  return s;
}

int Buffer::write_debug(int fd) {
  std::string s = debug_string();
  return ::write(fd, s.c_str(), s.length());
}

int Buffer::write_debug() {
  return write_debug(STDERR_FILENO);
}

/* Ensure implementations for various types are produced. */
template uint64_t Buffer::read<uint64_t>();
template bool Buffer::read<bool>();
template int Buffer::read<int>();
template uint32_t Buffer::write<uint64_t>(uint64_t);
template uint32_t Buffer::write<bool>(bool);
template uint32_t Buffer::write<int>(int);

} // namespace opendoor
