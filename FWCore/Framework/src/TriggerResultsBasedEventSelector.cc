#include <algorithm>

#include "FWCore/Framework/interface/TriggerResultsBasedEventSelector.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Algorithms.h"

static const edm::TypeID s_TrigResultsType(typeid(edm::TriggerResults));

namespace trigger_results_based_event_selector_utils {
  //--------------------------------------------------------
  // Remove whitespace (spaces and tabs) from a std::string.
  void remove_whitespace(std::string& s) {
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\t'), s.end());
  }

  //--------------------------------------------------------
  // Given a path-spec (std::string of the form "a:b", where the ":b" is
  // optional), return a parsed_path_spec_t containing "a" and "b".

  typedef std::pair<std::string, std::string> parsed_path_spec_t;
  void parse_path_spec(std::string const& path_spec, parsed_path_spec_t& output) {
    std::string trimmed_path_spec(path_spec);
    remove_whitespace(trimmed_path_spec);

    std::string::size_type colon = trimmed_path_spec.find(':');
    if (colon == std::string::npos) {
      output.first = trimmed_path_spec;
    } else {
      output.first = trimmed_path_spec.substr(0, colon);
      output.second = trimmed_path_spec.substr(colon + 1, trimmed_path_spec.size());
    }
  }
}  // namespace trigger_results_based_event_selector_utils

using namespace trigger_results_based_event_selector_utils;

namespace edm {

  namespace detail {

    bool configureEventSelector(edm::ParameterSet const& iPSet,
                                std::string const& iProcessName,
                                std::vector<std::string> const& iAllTriggerNames,
                                edm::detail::TriggerResultsBasedEventSelector& oSelector,
                                edm::ConsumesCollector&& iC) {
      // If selectevents is an emtpy ParameterSet, then we are to write
      // all events, or one which contains a vstrig 'SelectEvents' that
      // is empty, we are to write all events. We have no need for any
      // EventSelectors.
      if (iPSet.empty()) {
        oSelector.setupDefault();
        return true;
      }

      std::vector<std::string> path_specs = iPSet.getParameter<std::vector<std::string> >("SelectEvents");

      if (path_specs.empty()) {
        oSelector.setupDefault();
        return true;
      }

      // If we get here, we have the possibility of having to deal with
      // path_specs that look at more than one process.
      std::vector<parsed_path_spec_t> parsed_paths(path_specs.size());
      for (size_t i = 0; i < path_specs.size(); ++i) {
        parse_path_spec(path_specs[i], parsed_paths[i]);
      }
      oSelector.setup(parsed_paths, iAllTriggerNames, iProcessName, std::move(iC));

      return false;
    }

    // typedef detail::NamedEventSelector NES;

    TriggerResultsBasedEventSelector::TriggerResultsBasedEventSelector() : selectors_(), wantAllEvents_(false) {}

    void TriggerResultsBasedEventSelector::setupDefault() { wantAllEvents_ = true; }

    void TriggerResultsBasedEventSelector::setup(std::vector<parsed_path_spec_t> const& path_specs,
                                                 std::vector<std::string> const& triggernames,
                                                 std::string const& process_name,
                                                 ConsumesCollector&& iC) {
      // paths_for_process maps each PROCESS names to a sequence of
      // PATH names
      std::map<std::string, std::vector<std::string> > paths_for_process;
      for (auto const& path_spec : path_specs) {
        // Default to current process if none specified
        if (path_spec.second.empty()) {
          paths_for_process[process_name].push_back(path_spec.first);
        } else {
          paths_for_process[path_spec.second].push_back(path_spec.first);
        }
      }
      // Now go through all the PROCESS names, and create a
      // NamedEventSelector for each.
      for (auto const& path : paths_for_process) {
        // For the current process we know the trigger names
        // from the configuration file
        if (path.first == process_name) {
          selectors_.emplace_back(path.first, EventSelector(path.second, triggernames), std::move(iC));
        } else {
          // For previous processes we do not know the trigger
          // names yet.
          selectors_.emplace_back(path.first, EventSelector(path.second), std::move(iC));
        }
      }
    }

    bool TriggerResultsBasedEventSelector::wantEvent(EventForOutput const& ev) {
      if (wantAllEvents_) {
        return true;
      }
      for (auto& selector : selectors_) {
        Handle<TriggerResults> handle;
        ev.getByToken<TriggerResults>(selector.token(), handle);
        bool match = selector.match(*handle);
        if (match) {
          return true;
        }
      }
      return false;
    }

    ParameterSetID registerProperSelectionInfo(
        edm::ParameterSet const& iInitial,
        std::string const& iLabel,
        std::map<std::string, std::vector<std::pair<std::string, int> > > const& outputModulePathPositions,
        bool anyProductProduced) {
      ParameterSet selectEventsInfo;
      selectEventsInfo.copyForModify(iInitial);
      selectEventsInfo.addParameter<bool>("InProcessHistory", anyProductProduced);
      std::vector<std::string> endPaths;
      std::vector<int> endPathPositions;

      if (!iLabel.empty()) {
        std::map<std::string, std::vector<std::pair<std::string, int> > >::const_iterator iter =
            outputModulePathPositions.find(iLabel);
        assert(iter != outputModulePathPositions.end());
        for (auto const& item : iter->second) {
          endPaths.push_back(item.first);
          endPathPositions.push_back(item.second);
        }
      }
      selectEventsInfo.addParameter<std::vector<std::string> >("EndPaths", endPaths);
      selectEventsInfo.addParameter<std::vector<int> >("EndPathPositions", endPathPositions);
      if (!selectEventsInfo.exists("SelectEvents")) {
        selectEventsInfo.addParameter<std::vector<std::string> >("SelectEvents", std::vector<std::string>());
      }
      selectEventsInfo.registerIt();

      return selectEventsInfo.id();
    }

  }  // namespace detail
}  // namespace edm
