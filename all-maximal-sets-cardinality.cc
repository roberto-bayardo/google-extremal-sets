// Copyright 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ---
// An algorithm for finding all maximal sets based on the cardinality
// property.
// ---
// Author: Roberto Bayardo

#include "all-maximal-sets-cardinality.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "data-source-iterator.h"
#include "set-properties.h"

namespace google_extremal_sets {

namespace {

// Returns true if the elements in set #2 are all contained by
// set #1.
inline bool DoesSubsume(
    std::vector<uint32_t>::const_iterator it1,
    const std::vector<uint32_t>::const_iterator it1_end,
    const uint32_t* it2,
    const uint32_t* const it2_end) {
  while (it2 != it2_end) {
    it1 = std::lower_bound(it1, it1_end, *it2);
    if (it1 == it1_end || *it1 > *it2)
      return false;
    ++it1;
    ++it2;
  }
  return true;
}

}  // namespace

bool AllMaximalSetsCardinality::FindAllMaximalSets(
    DataSourceIterator* data,
    uint32_t max_item_id,
    uint32_t max_items_in_ram,
    OutputModeEnum output_mode) {
  Init();

  // The index_us vector contains the previous itemsets whose
  // cardinality is the same as the current itemset. We delay their
  // indexing until they can potentially be subsumed; that is, when
  // the data iterator reaches itemsets with a higher cardinality.
  std::vector<SetProperties*> index_us;

  // Vars set by the data source iterator.
  int result;
  uint32_t set_id;
  std::vector<uint32_t> current_set;

  // This outer loop supports multiple passes over the data in the
  // case where the dataset exceeds the bound on max_items_in_ram. As
  // long as resume_offset == 0, we will continue retaining itemsets
  // in RAM. Otherwise itemsets from the data iterator will be used
  // only to perform subsumption checks against existing candidates,
  // and will be indexed during a subsequent pass.
  off_t resume_offset = 0;
  do {  // while (resume_offset != 0)
    if (!PrepareForDataScan(data, max_item_id, resume_offset))
      return false;  // IO error
    resume_offset = 0;
    uint32_t items_in_ram = 0;
    int current_set_size = -1;

    // This loop scans the input data from beginning to end.
    while ((result = data->Next(&set_id, &current_set)) > 0) {
      DeleteSubsumedCandidates(current_set);

      // If current_set has higher cardinality than the itemsets within
      // index_us, we move them from index_us into the candidate index.
      if (current_set.size() != current_set_size) {
        IndexSets(index_us);
        index_us.clear();
        current_set_size = current_set.size();
      }

      if (resume_offset == 0) {
        // Copy the current_set into RAM and place a pointer to it in index_us.
        index_us.push_back(SetProperties::Create(set_id, current_set));
        items_in_ram += current_set.size();
        ++input_sets_count_;

        // Check if we've exceeded the RAM limit and if so stop
        // retaining any further itemsets in memory until the next
        // scan.
        if (items_in_ram >= max_items_in_ram) {
          resume_offset = data->Tell();
          std::cerr << "; Halting indexing at input set number "
                    << input_sets_count_ << " with id " << set_id << std::endl;
          // Force the sets in index_us to get added to the index.
          current_set_size = -1;
        }
      }  // if (resume_offset = 0)
    }  // while ((result = data->Next())

    if (result != 0)  // IO error
      return false;

    // At this point, any remaining candidate set and any remaining set
    // in index_us is maximal!
    DumpMaximalSets(&index_us, output_mode);
  } while (resume_offset != 0);

  return true;
}

void AllMaximalSetsCardinality::Init() {
  maximal_sets_count_ = input_sets_count_ = subsumption_checks_count_ = 0;
}

bool AllMaximalSetsCardinality::PrepareForDataScan(
    DataSourceIterator* data, int max_item_id, off_t resume_offset) {
  assert(candidates_.size() == 0);
  candidates_.resize(max_item_id);
  std::cerr << "; Starting new dataset scan at offset: "
            << resume_offset << std::endl;
  return data->Seek(resume_offset);
}

void AllMaximalSetsCardinality::IndexSets(const std::vector<SetProperties*>& index_us) {
  for (std::vector<SetProperties*>::const_iterator it = index_us.begin();
       it != index_us.end();
       ++it) {
    SetProperties* itemset_to_index = *it;
    uint32_t item_id_to_index = itemset_to_index->item[0];
    if (item_id_to_index >= candidates_.size())
      candidates_.resize(item_id_to_index + 1);
    candidates_[item_id_to_index].push_back(itemset_to_index);
  }
}

inline SetProperties* AllMaximalSetsCardinality::NextCandidate(
    const CandidateList& candidates,
    const std::vector<uint32_t>& current_set,
    int current_index,
    int* candidate_index) {
  do {
    ++(*candidate_index);
    if (*candidate_index == candidates.size())
      return 0;
  } while (!candidates[*candidate_index]);

  SetProperties* candidate = candidates[*candidate_index];
  if (current_set.size() - current_index < candidate->size)
    return 0;  // remaining sets are too big to be subsumed
  return candidate;
}

void AllMaximalSetsCardinality::DeleteSubsumedCandidates(
    const std::vector<uint32_t>& current_set) {
  std::vector<uint32_t>::const_iterator current_begin = current_set.begin();
  std::vector<uint32_t>::const_iterator current_end = current_set.end();
  SetProperties* candidate = 0;
  for (int i = 0; i < current_set.size(); ++i, ++current_begin) {
    if (candidates_.size() <= current_set[i])
      return;
    CandidateList& candidates = candidates_[current_set[i]];
    int candidate_index = -1;
    while (candidate =
           NextCandidate(
               candidates,
               current_set,
               i,
               &candidate_index)) {
      // We must explicitly check subsumption. We need not check every
      // item in each set because we already know:
      // (1) the candidate does not contain any items within
      // current_set[0] through current_set[i - 1].
      // (2) the candidate's first item is already known to be
      // the same as current_set[i].
      // We adjust the iterators accordingly.
      if (DoesSubsume(
	      current_begin, current_end,
	      candidate->begin() + 1, candidate->end())) {
        // Candidate is not maximal, so we delete it. Note that we
        // must preserve the cardinality based ordering, so we NULL
        // out the pointer to the deleted entry rather than performing
        // any swapping. TODO: Occasionally it might be beneficial to
        // compress out the holes left by the NULL entries if we have
        // accumulated a significant number of them.
        candidates[candidate_index] = 0;
	SetProperties::Delete(candidate);
      }
      ++subsumption_checks_count_;
    }
  }
}

void AllMaximalSetsCardinality::DumpMaximalSets(
    std::vector<SetProperties*>* unindexed_sets,
    OutputModeEnum output_mode) {
  for (int i = 0; i < unindexed_sets->size(); ++i) {
    SetProperties* maximal_set = (*unindexed_sets)[i];
    FoundMaximalSet(*maximal_set, output_mode);
    SetProperties::Delete(maximal_set);
  }
  unindexed_sets->clear();
  for (int i = 0; i < candidates_.size(); ++i) {
    CandidateList& candidate_set = candidates_[i];
    for (int j = 0; j < candidate_set.size(); ++j) {
      SetProperties* maximal_set = candidate_set[j];
      if (maximal_set) {
        FoundMaximalSet(*maximal_set, output_mode);
	SetProperties::Delete(maximal_set);
      }
    }
    candidate_set.clear();
  }
  candidates_.clear();
  std::cout << std::flush;
}

void AllMaximalSetsCardinality::FoundMaximalSet(
    const SetProperties& maximal_set, OutputModeEnum output_mode) {
  ++maximal_sets_count_;
  switch (output_mode) {
    case COUNT_ONLY:
      break;
    case ID:
      std::cout << maximal_set.set_id << '\n';
      break;
    case ID_AND_ITEMS:
      std::cout << maximal_set << '\n';
      break;
    default:
      std::cout << "Huh?\n";
      assert(0);
      break;
  }
}

}  // namespace google_extremal_sets
