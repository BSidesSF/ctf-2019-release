#ifndef _CRYPTER_H_
#define _CRYPTER_H_

#include <memory>
#include <cstdint>

#include "buffer.h"

namespace opendoor {

  class Crypter {
    public:
      virtual std::shared_ptr<Buffer> decrypt(std::shared_ptr<Buffer>) = 0;
      virtual std::shared_ptr<Buffer> encrypt(std::shared_ptr<Buffer>) = 0;
  }; // class Crypter

  class PlainCrypter : public Crypter {
    public:
      PlainCrypter();
      PlainCrypter(bool);
      virtual std::shared_ptr<Buffer> decrypt(std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Buffer> encrypt(std::shared_ptr<Buffer>);
    private:
      bool debug_;
  }; // class PlainCrypter

  class AESCrypter : public Crypter {
    public:
      AESCrypter();
      AESCrypter(uint8_t *, uint8_t *);
      virtual std::shared_ptr<Buffer> decrypt(std::shared_ptr<Buffer>);
      virtual std::shared_ptr<Buffer> encrypt(std::shared_ptr<Buffer>);

    private:
      uint8_t *key_;
      uint8_t *iv_;
  }; // class AESCrypter

} // namespace opendoor

#endif
