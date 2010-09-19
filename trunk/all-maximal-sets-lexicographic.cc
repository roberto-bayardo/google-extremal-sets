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
// An algorithm for finding all maximal sets based on the
// lexicographic property.
// ---
// Author: Roberto Bayardo

#include "all-maximal-sets-lexicographic.h"
#include <assert.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include "data-source-iterator.h"
#include "set-properties.h"

namespace google_extremal_sets {

namespace {

// Perform a binary search to find the first non-NULL candidate in the
// range such that comp(current_item, candidate[depth]) no longer holds.
template<class Compare>
AllMaximalSetsLexicographic::CandidateList::iterator find_new_it(
    AllMaximalSetsLexicographic::CandidateList::iterator first,
    AllMaximalSetsLexicographic::CandidateList::iterator last,
    uint32_t current_item,
    unsigned int depth,
    Compare comp) {
  while (first != last && !*first)
    ++first;
  int len = last - first;
  int half;
  AllMaximalSetsLexicographic::CandidateList::iterator current;
  while (len > 0) {
    half = len >> 1;
    current = first + half;
    while (current < last && !*current)
      ++current;
    if (current == last) {
      len = half;
    } else if (comp(current_item, (**current)[depth])) {
      // Not far enough along yet!
      first += half + 1;
      len = len - half - 1;
      while (first < last && !*first) {
	++first;
	--len;
      }
      if (first == last)
	return last;
    } else {
      // We may be too far along.
      len = half;
    }
  }
  assert(*first);
  return first;
}

}  // namespace

bool AllMaximalSetsLexicographic::FindAllMaximalSets(
    DataSourceIterator* data, uint32_t max_item_id) {
  Init();

  // Vars set by the data source iterator.
  int result;
  uint32_t set_id;
  ItemSet current_set;
  ItemSet previous_set;

  // This outer loop supports multiple passes over the data in the
  // case where the dataset exceeds the bound on max_items_in_ram_. As
  // long as resume_offset == 0, we will continue retaining itemsets
  // in RAM.
  off_t start_offset = 0;
  off_t resume_offset = 0;
  do {  // while (resume_offset != 0)
    if (!PrepareForDataScan(data, max_item_id, resume_offset))
      return false;  // IO error
    start_offset = resume_offset;
    resume_offset = 0;
    items_in_ram_ = 0;
    // This loop scans the input data from beginning to end.
    while ((result = data->Next(&set_id, &current_set)) > 0) {
      if (resume_offset == 0) {
        // Copy the current_set into RAM and place a pointer to it in index_
        // if necessary. Replace any previous sets that are prefixes of the
        // current one.
        bool previous_subsumed;
	do {
	  previous_subsumed = false;
	  if (candidates_.size() &&
	      candidates_.back()->size < current_set.size()) {
	    SetProperties* previous = candidates_.back();
	    previous_subsumed = true;
	    for (unsigned int i = 0; i < previous->size; ++i) {
	      if (previous->item[i] != current_set[i]){
		previous_subsumed = false;
		break;
	      }
	    }
	  }
	  if (previous_subsumed) {
	    SetProperties* previous_candidate =
	      candidates_[candidates_.size() - 1];
	    items_in_ram_ -= previous_candidate->size;
	    SetProperties::Delete(previous_candidate);
	    candidates_.resize(candidates_.size() - 1);
	  }
	} while (previous_subsumed);
	candidates_.push_back(SetProperties::Create(set_id, current_set));
	if (current_set[0] >= index_.size()) {
	  unsigned int old_size = index_.size();
	  index_.resize(current_set[0] + 1);
	  for (unsigned int i = old_size; i <= current_set[0]; ++i) {
	    index_[i] = candidates_.size() - 1;
	  }
	}
        items_in_ram_ += current_set.size();
        ++input_sets_count_;
        // Check if we've exceeded the RAM limit and if so stop
        // retaining any further itemsets in memory until the next
        // scan.
        if (items_in_ram_ >= max_items_in_ram_) {
          resume_offset = data->Tell();
          std::cerr << "; Halting indexing at input set number "
                    << input_sets_count_ << " with id " << set_id << std::endl;
	  result = 0;
	  break;
        }
	previous_set = current_set;
      }
    }  // while ((result = data->Next() ...

    if (result != 0)  // IO error
      return false;

    std::cerr << "; Potential maximal sets: " << candidates_.size() << '\n'
	      << "; Beginning subsumption checking scan." << std::endl;
    for (unsigned int i = 0; i < candidates_.size() - 1; ++i) {
      if (candidates_[i])  // check to make sure not already deleted.
	DeleteSubsumedCandidates(i);
    }
    if (start_offset != 0) {
      if (!PrepareForDataScan(data, max_item_id, 0))
        return false;  // IO error
      while (data->Tell() < start_offset &&
             (result = data->Next(&set_id, &current_set)) > 0) {
        DeleteSubsumedCandidates(current_set);
      }
      if (result < 0)  // IO error
        return false;
    }
    std::cerr << "; Dumping maximal sets." << std::endl;
    DumpMaximalSets();
  } while (resume_offset != 0);
  return true;
}

void AllMaximalSetsLexicographic::Init() {
  maximal_sets_count_ = input_sets_count_ = canidate_seek_count_ = 0;
  std::cerr << "; Finding all maximal itemsets.\n"
            << "; Limit on number of items in main memory: "
            << max_items_in_ram_ << std::endl;
}

bool AllMaximalSetsLexicographic::PrepareForDataScan(
    DataSourceIterator* data, uint32_t max_item_id, off_t resume_offset) {
  assert(candidates_.size() == 0);
  index_.reserve(max_item_id + 1);
  std::cerr << "; Starting new dataset scan at offset: "
            << resume_offset << std::endl;
  return data->Seek(resume_offset);
}

void AllMaximalSetsLexicographic::DeleteSubsumedCandidates(unsigned int current_set_index) {
  current_set_ = candidates_[current_set_index];
  assert(current_set_);
  // If the set is of size 1 it cannot subsume anything (assuming no
  // dupes)
  if (current_set_->size <= 1)
    return;

  const uint32_t* current_set_it = current_set_->begin();
  // The first candidate_set we consider is the first set following
  // current_set in the ordering, if one exists.
  CandidateList::iterator begin_range_it =
      candidates_.begin() + current_set_index + 1;
  DeleteSubsumedFromRange(begin_range_it, candidates_.end(), current_set_it, 0);
}

void AllMaximalSetsLexicographic::DeleteSubsumedCandidates(const ItemSet& itemset) {
  if (itemset.size() <= 1)
    return;
  current_set_ = SetProperties::Create(0, itemset);

  const uint32_t* current_set_it = current_set_->begin();
  DeleteSubsumedFromRange(
      candidates_.begin(), candidates_.end(), current_set_it, 0);
  SetProperties::Delete(current_set_);
}

// Helper function that advances begin_range_it over all subsumed &
// already deleted candidate sets, and deletes all subsumed itemsets
// encountered.
inline void AllMaximalSetsLexicographic::DeleteSubsumedSets(
    CandidateList::iterator* begin_range_it,
    CandidateList::iterator end_range_it,
    unsigned int depth) {
  // If current_set_->size() == depth, then the current set
  // cannot *properly* subsume any candidates.
  if (current_set_->size > depth) {
    while (*begin_range_it != end_range_it &&
           (!**begin_range_it || (**begin_range_it)->size == depth)) {
      if (**begin_range_it) {
        // Subsumed!
        items_in_ram_ -= (**begin_range_it)->size;
        SetProperties::Delete(**begin_range_it);
        **begin_range_it = 0;
      }
      ++(*begin_range_it);
      }
  } else {
    // Otherwise just skip over already-deleted itemsets.
    while (*begin_range_it != end_range_it && !**begin_range_it) {
      ++(*begin_range_it);
    }
  }
}

inline
AllMaximalSetsLexicographic::CandidateList::iterator AllMaximalSetsLexicographic::GetNewBeginRangeIt(
    CandidateList::iterator begin_range_it,
    CandidateList::iterator end_range_it,
    uint32_t current_item,
    unsigned int depth) {
  ++canidate_seek_count_;
  if (depth == 0) {
    // At depth 0 we can use the index rather than binary search.
    if (current_item >= index_.size())
      return end_range_it;
    if (candidates_.begin() + index_[current_item] > begin_range_it) {
      begin_range_it = candidates_.begin() + index_[current_item];
    }
    while (begin_range_it != end_range_it && !*begin_range_it)
      ++begin_range_it;
  } else {
    begin_range_it = find_new_it(
        begin_range_it,
        end_range_it,
        current_item,
        depth,
        std::greater<uint32_t>());
  }
  return begin_range_it;
}

inline AllMaximalSetsLexicographic::CandidateList::iterator AllMaximalSetsLexicographic::GetNewEndRangeIt(
    CandidateList::iterator begin_range_it,
    CandidateList::iterator end_range_it,
    uint32_t current_item,
    unsigned int depth) {
  ++canidate_seek_count_;
  CandidateList::iterator new_end_range_it;
  if (depth == 0) {
    // At depth 0 we can use the index rather than binary search.
    if (current_item + 1 < index_.size()) {
      new_end_range_it = candidates_.begin() + index_[current_item + 1];
      assert(new_end_range_it <= end_range_it);
    } else {
      new_end_range_it = end_range_it;
    }
  } else {
    new_end_range_it = find_new_it(
        begin_range_it,
        end_range_it,
        current_item,
        depth,
        std::equal_to<uint32_t>());
  }
  return new_end_range_it;
}

// This function has 2 important preconditions:
//   (1) all candidates between begin_range_it and end_range_it have
//   the same length-d prefix where d is the value of "depth"
//   (2) *current_set_it <= candidate[d+1] for any candidate with more
//   than d elements.
void AllMaximalSetsLexicographic::DeleteSubsumedFromRange(
    CandidateList::iterator begin_range_it,
    CandidateList::iterator end_range_it,
    const uint32_t* current_set_it,
    unsigned int depth) {
  assert(begin_range_it != end_range_it);
  DeleteSubsumedSets(&begin_range_it, end_range_it, depth);
  if (begin_range_it == end_range_it || current_set_it == current_set_->end())
    return;

  do {  // while (begin_range_it != end_range_it)
    // First thing we do is find the next item in the current_set
    // that, if added to our prefix, could potentially subsume some
    // candidate within the remaining range.
    uint32_t candidate_item = (**begin_range_it)[depth];
    assert(current_set_it != current_set_->end());
    if (*current_set_it < candidate_item) {
      current_set_it = std::lower_bound(
          current_set_it, current_set_->end(), candidate_item);
    }
    if (current_set_it == current_set_->end())
      return;
    assert(*current_set_it >= candidate_item);

    if (*current_set_it == candidate_item) {
      // The item we found matches the next candidate set item, which
      // means we can extend the prefix. Before we recurse, we must
      // compute an end range for the extended prefix.
      CandidateList::iterator new_end_range_it = GetNewEndRangeIt(
          begin_range_it, end_range_it, candidate_item, depth);
      assert(new_end_range_it >= begin_range_it);
      if (begin_range_it != new_end_range_it) {
        DeleteSubsumedFromRange(
            begin_range_it, new_end_range_it, current_set_it + 1, depth + 1);
      }
      begin_range_it = new_end_range_it;
      while (begin_range_it != end_range_it && !*begin_range_it)
        ++begin_range_it;
    } else {
      // Advance the begin_range until we reach potentially subsumable candidates.
      begin_range_it = GetNewBeginRangeIt(
          begin_range_it, end_range_it, *current_set_it, depth);
    }
  } while (begin_range_it != end_range_it);
}

void AllMaximalSetsLexicographic::DumpMaximalSets() {
  for (unsigned int i = 0; i < candidates_.size(); ++i) {
    SetProperties* maximal_set = candidates_[i];
    if (maximal_set) {
      FoundMaximalSet(*maximal_set);
      SetProperties::Delete(maximal_set);
    }
  }
  candidates_.clear();
  std::cout << std::flush;
}

void AllMaximalSetsLexicographic::FoundMaximalSet(const SetProperties& maximal_set) {
  ++maximal_sets_count_;
  switch (output_mode_) {
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

}  // google_extremal_sets
