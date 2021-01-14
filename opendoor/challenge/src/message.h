#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <functional>
#include <memory>
#include <string>
#include <map>
#include <cstdint>

#include "buffer.h"
#include "state.h"

namespace opendoor {

  class Message {
    public:
      static std::shared_ptr<Message> ParseMessage(
          const std::shared_ptr<Buffer>);
      virtual bool parse_message(const std::shared_ptr<Buffer>) = 0;
      virtual std::shared_ptr<Message> execute(std::shared_ptr<State>);
      virtual std::string to_string() = 0;
      virtual std::shared_ptr<Buffer> serialize();
      virtual std::shared_ptr<Message> ptr();
      virtual uint32_t get_id() = 0;

    private:
      // map of message code -> class constructor
      static std::map<std::uint32_t, std::function<
        std::shared_ptr<Message>()>> type_map_;
  }; // class Message

  class PingRequestMessage : public Message {
    public:
      PingRequestMessage(){}
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Message> execute(std::shared_ptr<State>);
      virtual std::string to_string();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x00000001;
  }; // class PingRequestMessage

  class PingResponseMessage : public Message {
    public:
      PingResponseMessage(){}
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::string to_string();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x00000002;
  }; // class PingResponseMessage

  class UnlockRequestMessage : public Message {
    public:
      UnlockRequestMessage();
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Message> execute(std::shared_ptr<State>);
      virtual std::string to_string();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x00000003;

    private:
      uint64_t badge_num_;
      uint64_t door_num_;
  }; // class UnlockRequestMessage

  class UnlockResponseMessage : public Message {
    public:
      UnlockResponseMessage();
      UnlockResponseMessage(
          uint64_t badge_num, uint64_t door_num, bool success);
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::string to_string();
      virtual std::shared_ptr<Buffer> serialize();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x00000004;

    private:
      uint64_t badge_num_;
      uint64_t door_num_;
      bool success_;
  }; // class UnlockResponseMessage

  enum DebugOperation {
    DBG_INVALID,
    DBG_ENABLE,
    DBG_READFILE,
  }; // enum DebugOperation

  class DebugRequestMessage : public Message {
    public:
      DebugRequestMessage();
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Message> execute(std::shared_ptr<State>);
      virtual std::string to_string();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x00000005;

    private:
      DebugOperation op_;
      bool bool_data_;
      std::string string_data_;

      std::shared_ptr<Message> handle_debug_message_(std::shared_ptr<State>);
      std::shared_ptr<Message> handle_readfile_(std::shared_ptr<State>);
  }; // class DebugRequestMessage

  class DebugResponseMessage : public Message {
    public:
      DebugResponseMessage();
      DebugResponseMessage(DebugOperation op);
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::string to_string();
      virtual std::shared_ptr<Buffer> serialize();
      virtual uint32_t get_id() {return ID;};

      void set_bool(bool val) { bool_data_ = val; }
      void set_string(std::string val) { string_data_ = val; }

      static const uint32_t ID = 0x00000006;

    private:
      DebugOperation op_;
      bool bool_data_;
      std::string string_data_;
  }; // class DebugResponseMessage

  class ErrorMessage : public Message {
    public:
      ErrorMessage();
      ErrorMessage(std::string);
      ErrorMessage(std::string, uint32_t);
      ErrorMessage(uint32_t);
      virtual bool parse_message(const std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Message> execute(std::shared_ptr<State>);
      virtual std::string to_string();
      virtual std::shared_ptr<Buffer> serialize();
      virtual uint32_t get_id() {return ID;};

      static const uint32_t ID = 0x0FFFFFFF;
      static const uint32_t Err_Unknown = 0;
      static const uint32_t Err_Msg = 1;
      static const uint32_t Err_Timestamp = 2;
      static const uint32_t Err_Parsing = 3;
      static const uint32_t Err_Crypto = 4;
      static const uint32_t Err_Access = 403;
      static const uint32_t Err_NotFound = 404;

    private:
      std::string err_;
      uint32_t err_code_;
      static std::map<uint32_t, std::string> message_map_;

      static std::string *string_for_code_(uint32_t);
  }; // class ErrorMessage

} // namespace opendoor

#endif
