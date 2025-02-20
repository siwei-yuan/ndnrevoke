#ifndef NDNREVOKE_APPEND_APPEND_COMMON_HPP
#define NDNREVOKE_APPEND_APPEND_COMMON_HPP

#include "revocation-common.hpp"
#include <ndn-cxx/util/random.hpp>

namespace ndnrevoke {

// CT is an append-only log, therefore we need a procotol that let producer Apps append

// Tianyuan: I copied it from the repo protocol
// AppendParameter =
//     [Name]
//     [ForwardingHint]
// Temporarily we don't consider segementation

// AppendResponse =
//     [Name]
//     [StatusCode]

// ForwardingHint = FORWARDING-HINT-TYPE TLV-LENGTH Name
// StatusCode = STATUS-CODE-TYPE TLV-LENGTH NonNegativeInteger

namespace tlv {

// notification
enum : uint32_t {
  AppenderPrefix = 261,
  AppenderNonce = 262
};

// submission
enum : uint32_t {
  AppendCert = 261,
  AppendRecord = 261,
};

enum : uint32_t {
  AppendParameters = 251,
  AppendStatusCode = 252
};

// Append Status
enum class AppendStatus : uint64_t {
  SUCCESS = 0,
  FAILURE_NACK = 1,
  FAILURE_TIMEOUT = 2,
  FAILURE_VALIDATION = 3,
  FAILURE_NX_CERT = 4,
  FAILURE_STORAGE = 98,
  NOTINITIALIZED = 99,
};

} // namespace tlv
} // namespace ndnrevoke

#endif // NDNREVOKE_APPEND_APPEND_COMMON_HPP
