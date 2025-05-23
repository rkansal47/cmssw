// -*- C++ -*-
//
// Package:     FWCore/AbstractServices
// Class  :     TimingServiceBase
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Wed, 11 Jun 2014 15:08:00 GMT
//

#include "FWCore/AbstractServices/interface/TimingServiceBase.h"

using namespace edm;

std::chrono::steady_clock::time_point TimingServiceBase::s_jobStartTime;

void TimingServiceBase::jobStarted() {
  if (0 == s_jobStartTime.time_since_epoch().count()) {
    s_jobStartTime = std::chrono::steady_clock::now();
  }
}

//
// constructors and destructor
//
TimingServiceBase::TimingServiceBase() = default;

TimingServiceBase::~TimingServiceBase() = default;
