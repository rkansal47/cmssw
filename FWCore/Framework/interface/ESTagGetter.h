#ifndef FWCore_Framework_ESTagGetter_h
#define FWCore_Framework_ESTagGetter_h
// -*- C++ -*-
//
// Package:     FWCore/Framework
// Class  :     ESTagGetter
//
/**\class ESTagGetter ESTagGetter.h "ESTagGetter.h"

 Description: Used with mayConsume option of ESConsumesCollector

 Usage:
    This contains a cache of data about all Resolvers which are
    associated with data produces which are in the Record and which
    are of the proper type.

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 19 Sep 2019 15:51:19 GMT
//

// system include files
#include <string_view>
#include <vector>

// user include files
#include "FWCore/Utilities/interface/ESIndices.h"

// forward declarations

namespace edm {
  namespace test {
    class ESTagGetterTester;
  }
  class ESTagGetter {
  public:
    friend test::ESTagGetterTester;
    struct Info {
      template <typename I, typename P, typename M>
      Info(I&& iIndex, P&& iProduct, M&& iModule)
          : index_(std::forward<I>(iIndex)),
            productLabel_(std::forward<P>(iProduct)),
            moduleLabel_(std::forward<M>(iModule)) {}
      ESResolverIndex index_;
      std::string productLabel_;
      std::string_view moduleLabel_;
    };

    ESTagGetter() = default;
    ESTagGetter(ESTagGetter const&) = default;
    ESTagGetter(ESTagGetter&&) = default;

    ESTagGetter(std::vector<Info> const& iLookup) : lookup_(iLookup) {}

    ESTagGetter& operator=(ESTagGetter const&) = default;
    ESTagGetter& operator=(ESTagGetter&&) = default;

    // ---------- const member functions ---------------------
    ESResolverIndex operator()(std::string_view iModuleLabel, std::string_view iProductLabel) const;

    ESResolverIndex nothing() const { return ESResolverIndex(); }

    ///Returns true if the Record being searched contains no products of the proper type
    bool hasNothingToGet() const { return lookup_.empty(); }

    std::vector<Info> const& lookup() const { return lookup_; }

  private:
    std::vector<Info> lookup_;
  };
}  // namespace edm

#endif
