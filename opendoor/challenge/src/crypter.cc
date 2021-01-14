#include "crypter.h"
#include "common.h"

#include <openssl/evp.h>
#include <openssl/err.h>

namespace opendoor {

static uint8_t def_key[] = {
  0x97, 0x8b, 0x8b, 0x8f, 0x8c, 0xc5, 0xd0, 0xd0,
  0x88, 0x88, 0x88, 0xd1, 0x8c, 0x86, 0x8c, 0x8b,
  0x9a, 0x92, 0x90, 0x89, 0x9a, 0x8d, 0x93, 0x90,
  0x8d, 0x9b, 0xd1, 0x9c, 0x90, 0x92, 0xd0, 0xff,
};

static uint8_t def_iv[] = {
  0x6e, 0x6f, 0x74, 0x61, 0x66, 0x6c, 0x61, 0x67,
  0x6e, 0x6f, 0x74, 0x61, 0x66, 0x6c, 0x61, 0x67,
};

static uint32_t block_ceiling(uint32_t in);

PlainCrypter::PlainCrypter()
  : debug_(false) {}

PlainCrypter::PlainCrypter(bool debug)
  : debug_(debug) {}

std::shared_ptr<Buffer> PlainCrypter::decrypt(std::shared_ptr<Buffer> in) {
  if (debug_) {
    DBG("Ciphertext:");
    in->write_debug();
  }
  auto raw = in->read<std::shared_ptr<std::vector<uint8_t>>>();
  auto rv = std::shared_ptr<Buffer>(new Buffer(raw));
  if (debug_) {
    DBG("Plaintext:");
    rv->write_debug();
  }
  return rv;
}

std::shared_ptr<Buffer> PlainCrypter::encrypt(std::shared_ptr<Buffer> in) {
  auto buf = new Buffer();
  buf->write(in->get_contents());
  if (debug_) {
    buf->write_debug();
  }
  return std::shared_ptr<Buffer>(buf);
}

AESCrypter::AESCrypter()
  : key_(def_key),
    iv_(def_iv)
  {}

AESCrypter::AESCrypter(uint8_t *key, uint8_t *iv)
  : key_(key),
    iv_(iv)
  {}

std::shared_ptr<Buffer> AESCrypter::decrypt(std::shared_ptr<Buffer> in) {
  auto raw = in->read<std::shared_ptr<std::vector<uint8_t>>>();
  auto in_len = raw->size();
  auto out_raw = new std::vector<uint8_t>(block_ceiling(in_len));
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  DBG("Input length: %lu", in_len);
  in->write_debug();
  if (!ctx)
    return nullptr;
  if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_, iv_)) {
    DBG("EVP_DecryptInit_ex failed!");
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  int out_len = 0;
  if (!EVP_DecryptUpdate(ctx, out_raw->data(), &out_len, raw->data(), in_len)) {
    DBG("EVP_DecryptUpdate failed!");
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  int pt_len = out_len;
  if (!EVP_DecryptFinal_ex(ctx, out_raw->data() + out_len, &out_len)) {
    DBG("EVP_DecryptFinal_ex failed!");
    ERR_print_errors_fp(stderr);
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  pt_len += out_len;
  EVP_CIPHER_CTX_free(ctx);
  out_raw->resize(pt_len);
  auto buf = std::make_shared<Buffer>(
      std::shared_ptr<std::vector<uint8_t>>(out_raw));
#ifdef DEBUG
  buf->write_debug();
#endif
  return buf;
}

std::shared_ptr<Buffer> AESCrypter::encrypt(std::shared_ptr<Buffer> in) {
  auto in_len = in->get_contents()->size();
  auto raw = in->get_contents()->data();
  auto out_raw = new std::vector<uint8_t>(block_ceiling(in_len));
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    return nullptr;
  if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_, iv_)) {
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  int out_len = 0;
  if (!EVP_EncryptUpdate(ctx, out_raw->data(), &out_len, raw, in_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  int ct_len = out_len;
  if (!EVP_EncryptFinal_ex(ctx, out_raw->data() + out_len, &out_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return nullptr;
  }
  ct_len += out_len;
  EVP_CIPHER_CTX_free(ctx);
  out_raw->resize(ct_len);
  auto out_buf = std::make_shared<Buffer>();
  out_buf->write<std::shared_ptr<std::vector<uint8_t>>>(
      std::shared_ptr<std::vector<uint8_t>>(out_raw));
  return out_buf;
}

static uint32_t block_ceiling(uint32_t in) {
  // round up to a block length (16)
  const int block_size = 16;
  auto remainder = in % block_size;
  if (remainder == 0)
    return in;
  return in + block_size - remainder;
}

} // namespace opendoor
