#include "append/append-common.hpp"
#include "append/handle-ct.hpp"

#include <ndn-cxx/security/signing-helpers.hpp>

namespace ndnrevoke {
namespace append {

NDN_LOG_INIT(ndnrevoke.append);


const ssize_t MAX_RETRIES = 2;

HandleCt::HandleCt(const ndn::Name& prefix, ndn::Face& face, ndn::KeyChain& keyChain)
  : Handle(prefix, face, keyChain)
{
}

std::shared_ptr<Data>
HandleCt::makeNotificationAck(const Name& notificationName, const tlv::AppendStatus status)
{
  auto data = std::make_shared<Data>(notificationName);
  // acking notification
  data->setContent(ndn::makeNonNegativeIntegerBlock(tlv::AppendStatusCode, static_cast<uint64_t>(status)));
  m_keyChain.sign(*data, ndn::signingByIdentity(m_localPrefix));
  return data;
}

void
HandleCt::dispatchInterest(const Interest& interest, const uint64_t nonce)
{
  auto item = m_nonceMap.find(nonce);
  if (item == m_nonceMap.end()) {
    return;
  }

  if (item->second.retryCount++ > MAX_RETRIES) {
    NDN_LOG_ERROR("Running out of retries: " << item->second.retryCount << " retries");
    // acking notification
    m_face.put(*makeNotificationAck(interest.getName(), tlv::AppendStatus::FAILURE_TIMEOUT));
    NDN_LOG_TRACE("Putting notification ack");
    m_nonceMap.erase(nonce);
    return;
  }
  
  m_face.expressInterest(interest, 
    [&] (auto& i, const auto& d) { onSubmissionData(i, d);},
    [&] (const auto& i, auto& n) {
      // acking notification
      m_face.put(*makeNotificationAck(interest.getName(), tlv::AppendStatus::FAILURE_NACK));
      NDN_LOG_TRACE("Putting notification ack");
      m_nonceMap.erase(nonce);
    },
    [&] (const auto& i) {
      NDN_LOG_TRACE("Retry");
      dispatchInterest(interest, nonce);
    }
  );
}

void
HandleCt::listenOnTopic(Name& topic, const UpdateCallback& onUpdateCallback)
{
  m_topic = topic;
  m_updateCallback = onUpdateCallback;
  if (m_topic.empty()) {
    NDN_LOG_TRACE("No topic to listen, return\n");
    return;
  }
  else {
    auto prefixId = m_face.registerPrefix(m_topic,[&] (const Name& name) {
      // register for each record Zone
      // notice: this only register FIB to Face, not NFD.
      auto filterId = m_face.setInterestFilter(Name(m_topic).append("notify"), [=] (auto&&, const auto& i) { onNotification(i); });
      m_interestFilterHandles.push_back(filterId);
      NDN_LOG_TRACE("Registering filter for notification " << Name(m_topic).append("notify"));
    },
    [] (auto&&, const auto& reason) { 
      NDN_LOG_ERROR("Failed to register prefix in local hub's daemon, REASON: " << reason);
    });
    m_registeredPrefixHandles.push_back(prefixId);
   }
}

void
HandleCt::onNotification(Interest interest)
{
  // Interest: <topic>/<nonce>/<paramDigest>
  // <topic> should be /<ct-prefix>/append
  appendtlv::AppenderInfo info;
  appendtlv::decodeAppendParameters(interest.getApplicationParameters(), info);
  NDN_LOG_TRACE("New notification: [nonce " << info.nonce << " ] [remotePrefix " << info.remotePrefix << " ]");
  info.interestName = interest.getName();
  info.retryCount = 0;
  m_nonceMap.insert({info.nonce, info});

  // send interst: /<remotePrefix>/msg/<topic>/<nonce>
  Interest submissionFetcher(Name(info.remotePrefix).append("msg").append(m_topic)
                                                    .appendNumber(info.nonce));
  if (!info.forwardingHint.empty()) {
    submissionFetcher.setForwardingHint({info.forwardingHint});
  }

  // ideally we need fill in all three callbacks
  dispatchInterest(submissionFetcher, info.nonce);
}

void
HandleCt::onSubmissionData(const Interest& interest, const Data& data)
{
  // /ndn/site1/abc/msg/ndn/append/%29%40%87u%89%F9%8D%E4
  auto content = data.getContent();
  const ssize_t NONCE_OFFSET = -1;
  const uint64_t nonce = data.getName().at(NONCE_OFFSET).toNumber();
  auto item = m_nonceMap.find(nonce);

  if (item != m_nonceMap.end()) {
    item->second.retryCount = 0;
    content.parse();
    for (const auto &item : content.elements()) {
      switch (item.type()) {
        case ndn::tlv::Data:
          // ideally we should run validator here
          m_updateCallback(Data(item));
          break;
        default:
          if (ndn::tlv::isCriticalType(item.type())) {
            NDN_THROW(std::runtime_error("Unrecognized TLV Type: " + std::to_string(item.type())));
          }
          else {
            //ignore
          }
          break;
      }
    }

    // acking notification
    m_face.put(*makeNotificationAck(interest.getName(), tlv::AppendStatus::SUCCESS));
    NDN_LOG_TRACE("Putting notification ack");
    m_nonceMap.erase(nonce);
  }
}

} // namespace append
} // namespace ndnrevoke
