#include "ct-module.hpp"

#include <iostream>
#include <chrono>
#include <deque>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>

namespace ndnrevoke {
namespace ct {

static ndn::Face face;
static ndn::KeyChain keyChain;

static int
main(int argc, char* argv[])
{
  try {
    keyChain.getPib().getIdentity(Name("/ndn"));
  }
  catch (const std::exception&) {
    keyChain.createIdentity(Name("/ndn"));
  }

  CtModule ct(face, keyChain, "ct.config.sample");
  face.processEvents();
  return 0;
}

} // namespace ct
} // namespace ndnrevoke

int
main(int argc, char* argv[])
{
  return ndnrevoke::ct::main(argc, argv);
}
