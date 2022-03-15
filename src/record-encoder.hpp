#ifndef NDNREVOKE_RECORD_ENCODER_HPP
#define NDNREVOKE_RECORD_ENCODER_HPP

#include "revocation-common.hpp"
#include "revocation-state.hpp"
#include <ndn-cxx/security/key-chain.hpp>

namespace ndnrevoke {
namespace recordtlv {

Block
encodeRecordContent(const std::vector<uint8_t>& publicKeyHash, const tlv::ReasonCode revocationReason);

void
decodeRecordContent(const Block& recordContent, state::State& state);

} // namespace recordtlv
} // namespace ndncert

#endif // NDNREVOKE_RECORD_ENCODER_HPP
