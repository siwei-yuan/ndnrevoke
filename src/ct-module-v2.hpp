#ifndef NDNREVOKE_CT_MODULE_V2_HPP
#define NDNREVOKE_CT_MODULE_V2_HPP

#include "storage/ct-storage-v2.hpp"
#include "append/handle-ct.hpp"
#include "ct-configuration.hpp"
#include "nack.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

namespace ndnrevoke {
namespace ct {

class CtModuleV2 : boost::noncopyable
{
public:
  CtModuleV2(ndn::Face& face, ndn::KeyChain& keyChain, const std::string& configPath,
           const std::string& storageType = "ct-storage-memory-v2");

  ~CtModuleV2();

  const std::unique_ptr<CtStorageV2>&
  getCtStorage()
  {
    return m_storage;
  }

  CtConfig&
  getCtConf()
  {
    return m_config;
  }

  void
  onQuery(const Interest& query);


NDNREVOKE_PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  tlv::AppendStatus onDataSubmission(const Data& data);

  std::shared_ptr<nack::Nack>
  prepareNack(const Name dataName, ndn::time::milliseconds freshnessPeriod);

  void
  registerPrefix();

  void
  onRegisterFailed(const std::string& reason);

  bool
  isValidQuery(Name queryName);

NDNREVOKE_PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::Face& m_face;
  CtConfig m_config;
  ndn::KeyChain& m_keyChain;
  std::unique_ptr<CtStorageV2> m_storage;
  std::list<ndn::RegisteredPrefixHandle> m_registeredPrefixHandles;
  std::list<ndn::InterestFilterHandle> m_interestFilterHandles;

  std::shared_ptr<append::HandleCt> m_handle;
};

} // namespace ct
} // namespace ndnrevoke

#endif // NDNREVOKE_CT_MODULE_V2_HPP
