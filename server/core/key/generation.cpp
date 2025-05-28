//
// Created by Jhean Lee on 2025/5/1.
//

#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "generation.hpp"
#include "../common/console.hpp"
#include "../common/shared.hpp"

int generate_ssl_key_cert(const std::string &root_path) {
  console(NOTICE, SSL_GENERATING_SSL_CREDENTIALS, nullptr, "key::generation::ssl");

  BIGNUM *bn = BN_new();
  if (BN_set_word(bn, RSA_F4) != 1) {
    console(CRITICAL, SSL_INIT_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }

  BIO *ssl_private_key_io = BIO_new_file((root_path + "/aqueduct-ssl-private.pem").c_str(), "w");
  BIO *ssl_certificate_io = BIO_new_file((root_path + "/aqueduct-ssl-cert").c_str(), "w");
  if (ssl_private_key_io == nullptr || ssl_certificate_io == nullptr) {
    console(CRITICAL, SSL_BIO_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }

  EVP_PKEY *pkey = EVP_RSA_gen(4096);
  if (pkey == nullptr) {
    console(CRITICAL, SSL_RSA_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }

  if (!PEM_write_bio_PrivateKey(ssl_private_key_io, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
    console(CRITICAL, SSL_KEY_WRITE_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }

  X509 *x509 = X509_new();
  if (x509 == nullptr) {
    console(CRITICAL, SSL_INIT_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);  //  one year
  X509_set_pubkey(x509, pkey);
  X509_NAME *name = X509_get_subject_name(x509);
  X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)"Aqueduct", -1, -1, 0);
  X509_set_issuer_name(x509, name);
  if (!X509_sign(x509, pkey, EVP_sha256())) {
    console(CRITICAL, SSL_CERT_SIGN_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }
  if (!PEM_write_bio_X509(ssl_certificate_io, x509)) {
    console(CRITICAL, SSL_CERT_WRITE_FAILED, nullptr, "key::generation::ssl");
    return 1;
  }

  BIO_free(ssl_private_key_io);
  BIO_free(ssl_certificate_io);

  return 0;
}

int generate_jwt_key_pair(const std::string &root_path, const std::string &prefix) {
  console(NOTICE, SSL_GENERATING_RSA_PAIR, nullptr, "key::generation::jwt");

  BIGNUM *bn = BN_new();
  if (BN_set_word(bn, RSA_F4) != 1) {
    console(CRITICAL, SSL_INIT_FAILED, nullptr, "key::generation::jwt");
    return 1;
  }

  //  path
  BIO *jwt_private_key_io = BIO_new_file((root_path + '/' + prefix + "-private.pem").c_str(), "w");
  BIO *jwt_public_key_io = BIO_new_file((root_path + '/' + prefix + "-public.pem").c_str(), "w");
  if (jwt_private_key_io  == nullptr || jwt_public_key_io == nullptr) {
    console(CRITICAL, SSL_BIO_FAILED, nullptr, "key::generation::jwt");
    return 1;
  }

  //  generation (RSA 4096)
  EVP_PKEY *pkey = EVP_RSA_gen(4096);
  if (pkey == nullptr) {
    console(CRITICAL, SSL_RSA_FAILED, nullptr, "key::generation::jwt");
    return 1;
  }

  //  write keys
  if (!PEM_write_bio_PrivateKey(jwt_private_key_io , pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
    console(CRITICAL, SSL_KEY_WRITE_FAILED, nullptr, "key::generation::jwt");
    return 1;
  }
  if (!PEM_write_bio_PUBKEY(jwt_public_key_io, pkey)) {
    console(CRITICAL, SSL_KEY_WRITE_FAILED, nullptr, "key::generation::jwt");
    return 1;
  }

  BIO_free(jwt_private_key_io);
  BIO_free(jwt_public_key_io);

  return 0;
}
