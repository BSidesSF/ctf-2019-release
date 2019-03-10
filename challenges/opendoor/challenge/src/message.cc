#include "common.h"
#include "message.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

namespace opendoor {
  bool time_in_window(time_t when, time_t range);

  std::shared_ptr<Message> Message::ParseMessage(
      const std::shared_ptr<Buffer> buf) {
    uint32_t msg_id = buf->read<uint32_t>();
    uint32_t tstamp = buf->read<uint32_t>();
    if (!time_in_window(tstamp, 5)) {
      // return error of time
      DBG("Error, incorrect timestamp.");
      return (new ErrorMessage(ErrorMessage::Err_Timestamp))->ptr();
    }
    auto entry = type_map_.find(msg_id);
    if (entry == type_map_.end()) {
      // return error of type
      DBG("Error finding message type.");
      return (new ErrorMessage(ErrorMessage::Err_Msg))->ptr();
    }
    std::shared_ptr<Message> msg = (entry->second)();
    if (!msg->parse_message(buf)) {
      // return error of parsing
      DBG("Error parsing message data.");
      return (new ErrorMessage(ErrorMessage::Err_Parsing))->ptr();
    }
    DBG("Parsed message: %s", msg->to_string().c_str());
    return msg;
  }

  std::shared_ptr<Buffer> Message::serialize() {
    auto buf = std::make_shared<Buffer>();
    buf->write(get_id());
    buf->write<uint32_t>(time(NULL));
    return buf;
  }

  std::shared_ptr<Message> Message::execute(std::shared_ptr<State> state) {
    return std::shared_ptr<Message>(nullptr);
  }

  std::shared_ptr<Message> Message::ptr() {
    return std::shared_ptr<Message>(this);
  }

  /* PingRequestMessage */
  bool PingRequestMessage::parse_message(const std::shared_ptr<Buffer>) {
    return true;
  }

  std::shared_ptr<Message> PingRequestMessage::execute(
      std::shared_ptr<State> state) {
    return std::shared_ptr<Message>(
        dynamic_cast<Message*>(new PingResponseMessage()));
  }

  std::string PingRequestMessage::to_string() {
    return "PingRequest";
  }

  /* PingResponseMessage */
  bool PingResponseMessage::parse_message(const std::shared_ptr<Buffer>) {
    return true;
  }

  std::string PingResponseMessage::to_string() {
    return "PingResponse";
  }

  /* ErrorMessage */
  ErrorMessage::ErrorMessage()
    : err_("Unknown Error"),
      err_code_(Err_Unknown)
  {}

  ErrorMessage::ErrorMessage(std::string err)
    : err_(err),
      err_code_(Err_Unknown)
  {}

  ErrorMessage::ErrorMessage(uint32_t code)
    : err_code_(code)
  {
    std::string *s = string_for_code_(code);
    if (s == NULL) {
      err_code_ = Err_Unknown;
      s = string_for_code_(err_code_);
    }
    err_ = *s;
  }

  ErrorMessage::ErrorMessage(std::string err, uint32_t code)
    : err_(err),
      err_code_(code)
  {}

  bool ErrorMessage::parse_message(const std::shared_ptr<Buffer> buf){
    err_code_ = buf->read<uint32_t>();
    err_ = buf->read<std::string>();
    return err_.length() > 0;
  }

  std::shared_ptr<Message> ErrorMessage::execute(std::shared_ptr<State> state){
    return std::shared_ptr<Message>(this);
  }

  std::string ErrorMessage::to_string(){
    return std::string("Error: ") + err_;
  }

  std::shared_ptr<Buffer> ErrorMessage::serialize(){
    auto buf = Message::serialize();
    buf->write<std::string*>(&err_);
    return buf;
  }

  std::string *ErrorMessage::string_for_code_(uint32_t code) {
    auto item = message_map_.find(code);
    if (item == message_map_.end()) {
      return NULL;
    }
    return &item->second;
  }

  /* UnlockRequestMessage */
  UnlockRequestMessage::UnlockRequestMessage()
    : badge_num_(0),
      door_num_(0)
  {}

  bool UnlockRequestMessage::parse_message(const std::shared_ptr<Buffer> buf) {
    badge_num_ = buf->read<uint64_t>();
    door_num_ = buf->read<uint64_t>();
    return true;
  }

  std::shared_ptr<Message> UnlockRequestMessage::execute(
      std::shared_ptr<State> state) {
    if (badge_num_ == 0 || door_num_ != state->get_door_num()) {
      state->lock();
      return (new ErrorMessage(
            "Request not valid for door.", ErrorMessage::Err_Access))->ptr();
    }
    state->unlock();
    auto resp = new UnlockResponseMessage(
        badge_num_, door_num_, state->is_unlocked());
    return resp->ptr();
  }

  std::string UnlockRequestMessage::to_string() {
    char buf[256];
    snprintf(buf, sizeof(buf), "UnlockRequestMessage: badge %lu, door %lu",
        badge_num_, door_num_);
    return std::string(buf);
  }

  /* UnlockResponseMessage */
  UnlockResponseMessage::UnlockResponseMessage()
    : badge_num_(0),
      door_num_(0),
      success_(false)
  {}

  UnlockResponseMessage::UnlockResponseMessage(
      uint64_t badge_num, uint64_t door_num, bool success)
    : badge_num_(badge_num),
      door_num_(door_num),
      success_(success)
  {}

  std::string UnlockResponseMessage::to_string() {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "UnlockResponseMessage: badge %lu, door %lu, success %s",
        badge_num_, door_num_, success_ ? "true" : "false");
    return std::string(buf);
  }

  std::shared_ptr<Buffer> UnlockResponseMessage::serialize(){
    auto buf = Message::serialize();
    buf->write(badge_num_);
    buf->write(door_num_);
    buf->write(success_);
    return buf;
  }

  bool UnlockResponseMessage::parse_message(const std::shared_ptr<Buffer> buf) {
    badge_num_ = buf->read<uint64_t>();
    door_num_ = buf->read<uint64_t>();
    success_ = buf->read<bool>();
    return true;
  }

  /* DebugRequestMessage */
  DebugRequestMessage::DebugRequestMessage()
    : op_(DBG_INVALID),
      bool_data_(false),
      string_data_()
  {}

  bool DebugRequestMessage::parse_message(const std::shared_ptr<Buffer> buf) {
    op_ = (DebugOperation)buf->read<int>();
    switch (op_) {
      case DBG_ENABLE:
        bool_data_ = buf->read<bool>();
        break;
      case DBG_READFILE:
        string_data_ = buf->read<std::string>();
        break;
      default:
        return false;
    }
    return true;
  }

  std::shared_ptr<Message> DebugRequestMessage::execute(
      std::shared_ptr<State> state) {
    if (op_ == DBG_ENABLE) {
      return handle_debug_message_(state);
    }
    if (!state->is_debuggable()) {
      return (new ErrorMessage(ErrorMessage::Err_Access))->ptr();
    }
    switch (op_) {
      case DBG_READFILE:
        return handle_readfile_(state);
      default:
        return (new ErrorMessage(ErrorMessage::Err_Unknown))->ptr();
    }
    return (new ErrorMessage(ErrorMessage::Err_Unknown))->ptr();
  }

  std::shared_ptr<Message> DebugRequestMessage::handle_debug_message_(
    std::shared_ptr<State> state) {
    auto resp = new DebugResponseMessage(op_);
    resp->set_bool(bool_data_);
    if (!bool_data_) {
      state->set_debuggable(false);
      return resp->ptr();
    } else if (state->is_unlocked()) {
      state->set_debuggable(true);
      return resp->ptr();
    } else {
      return (new ErrorMessage(ErrorMessage::Err_Access))->ptr();
    }
  }

  std::shared_ptr<Message> DebugRequestMessage::handle_readfile_(
      std::shared_ptr<State> state) {
    FILE *fp = fopen(string_data_.c_str(), "r");
    if (!fp)
      return (new ErrorMessage(ErrorMessage::Err_NotFound))->ptr();
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    size_t n = fread(buf, sizeof(char), sizeof(buf)/sizeof(char), fp);
    fclose(fp);

    auto resp = new DebugResponseMessage(op_);
    resp->set_string(std::string(buf, n));
    return resp->ptr();
  }

  std::string DebugRequestMessage::to_string() {
    std::string str("DebugRequestMessage ");
    switch (op_) {
      case DBG_ENABLE:
        if (bool_data_)
          str += "enable";
        else
          str += "disable";
        break;
      case DBG_READFILE:
        str += "read ";
        str += string_data_;
        break;
      default:
        str += "(UNKNOWN)";
    }
    return str;
  }

  /* DebugResponseMessage */
  DebugResponseMessage::DebugResponseMessage()
    : op_(DBG_INVALID),
      bool_data_(false),
      string_data_()
  {}

  DebugResponseMessage::DebugResponseMessage(DebugOperation op)
    : op_(op),
      bool_data_(false),
      string_data_()
  {}

  bool DebugResponseMessage::parse_message(const std::shared_ptr<Buffer> buf) {
    op_ = (DebugOperation)buf->read<int>();
    switch (op_) {
      case DBG_ENABLE:
        bool_data_ = buf->read<bool>();
        break;
      case DBG_READFILE:
        string_data_ = buf->read<std::string>();
        break;
      default:
        return false;
    }
    return true;
  }

  std::shared_ptr<Buffer> DebugResponseMessage::serialize() {
    auto buf = Message::serialize();
    buf->write((int)op_);
    switch (op_) {
      case DBG_ENABLE:
        buf->write(bool_data_);
        break;
      case DBG_READFILE:
        buf->write(&string_data_);
        break;
      default:
        return (new ErrorMessage(ErrorMessage::Err_Unknown))->serialize();
    }
    return buf;
  }

  std::string DebugResponseMessage::to_string() {
    std::string str("DebugResponseMessage ");
    switch (op_) {
      case DBG_ENABLE:
        if (bool_data_)
          str += "enabled";
        else
          str += "disabled";
        break;
      case DBG_READFILE:
        str += "read data: ";
        str += string_data_;
        break;
      default:
        str += "(UNKNOWN)";
    }
    return str;
  }

  /** Utility functions */

  // Check if time is within range of now.
  bool time_in_window(time_t when, time_t range) {
    time_t now = time(NULL);
    DBG("Now: %d, have %d", now, when);
    if ((now - range) > when)
      return false;
    if ((now + range) < when)
      return false;
    return true;
  }

  // Const definitions
  const uint32_t PingRequestMessage::ID;
  const uint32_t PingResponseMessage::ID;
  const uint32_t UnlockRequestMessage::ID;
  const uint32_t UnlockResponseMessage::ID;
  const uint32_t DebugRequestMessage::ID;
  const uint32_t DebugResponseMessage::ID;
  const uint32_t ErrorMessage::ID;
  const uint32_t ErrorMessage::Err_Unknown;
  const uint32_t ErrorMessage::Err_Msg;
  const uint32_t ErrorMessage::Err_Timestamp;
  const uint32_t ErrorMessage::Err_Parsing;
  const uint32_t ErrorMessage::Err_Crypto;
  const uint32_t ErrorMessage::Err_Access;
  const uint32_t ErrorMessage::Err_NotFound;

  // Factory map
  std::map<std::uint32_t, std::function<std::shared_ptr<Message>()>>
    Message::type_map_ = {
      {PingRequestMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new PingRequestMessage()));}},
      {PingResponseMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new PingResponseMessage()));}},
      {UnlockRequestMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new UnlockRequestMessage()));}},
      {UnlockResponseMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new UnlockResponseMessage()));}},
      {DebugRequestMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new DebugRequestMessage()));}},
      {DebugResponseMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new DebugResponseMessage()));}},
      {ErrorMessage::ID, [](){return std::shared_ptr<Message>(
          dynamic_cast<Message*>(new ErrorMessage()));}},
    };

  // Message map
  std::map<uint32_t, std::string> ErrorMessage::message_map_ = {
    {ErrorMessage::Err_Unknown, "Unknown"},
    {ErrorMessage::Err_Msg, "Invalid Message Type"},
    {ErrorMessage::Err_Timestamp, "Invalid Timestamp"},
    {ErrorMessage::Err_Parsing, "Error Parsing"},
    {ErrorMessage::Err_Access, "Access Denied"},
    {ErrorMessage::Err_Crypto, "Crypto Error"},
    {ErrorMessage::Err_NotFound, "Resource Not Found"},
  };

} // namespace opendoor
