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
//
// ---
// This class implements an algorithm that computes all maximal sets
// from a given input list of sets.
//
// The input list must have the following properties for the algorithm
// to behave correctly and/or efficiently:
//
// 1) Sets in the file are assumed to appear in increasing
// lexicographic order.
//
// 2) Items within a set must always appear from least to most
// frequent in a consistent order.  That is if item $x$ appears less
// frequently than item $y$ within the dataset, then $x$ should always
// appear before $y$ within any set containing both items. Furthermore
// if two items $x$ and $y$ have the same frequency, then one must be
// chosen to consistently appear before the other should they both
// appear in a given set.
//
// 3) A set must not contain duplicate items.
// ---
// Author: Roberto Bayardo
//
#ifndef _ALL_MAXIMAL_SETS_LEXICOGRAPHIC_H_
#define _ALL_MAXIMAL_SETS_LEXICOGRAPHIC_H_

#include <limits>
#include <vector>
#include "basic-types.h"

namespace google_extremal_sets {

class DataSourceIterator;
class SetProperties;

class AllMaximalSetsLexicographic {
 public:
  AllMaximalSetsLexicographic()
      : max_items_in_ram_(std::numeric_limits<uint32_t>::max()),
        output_mode_(ID) {
  }

  // Finds all maximal sets in the "data" stream. Does not assume
  // ownership of the data stream. Returns false if the computation
  // could not complete successfully because of a data stream error. A
  // call to data->GetErrorMessage() will return a human-readable
  // description of the problem.
  //
  // The caller must provide an estimate of the max_item_id which will
  // be used to preallocate buffers.  The correct output will be
  // produced even if the estimates are inaccurate (provided there is
  // sufficient memory for the buffers to be allocated to the
  // specified size.)
  //
  // This method may output status & progress messages to stderr.
  bool FindAllMaximalSets(DataSourceIterator* data, uint32_t max_item_id);

  // To specify a bound on the number of 4-byte item ids that will be
  // stored in main memory during algorithm execution. Should the
  // dataset contain more items than the limit, the algorithm will
  // switch to an "out of core" mode and perform multiple passes over
  // the data in order to compute the output. Default is to impose
  // no RAM limit.
  void SetMaxItemsInRam(int max) {
    max_items_in_ram_ = max;
  }

  // Set the output mode. Default is "ID".
  void SetOutputMode(OutputModeEnum mode) {
    output_mode_ = mode;
  }

  // Returns the number of maximal sets found by the last call to
  // FindAllMaximalSets.
  long MaximalSetsCount() const { return maximal_sets_count_; }

  // Returns the number of itemssets encountered in the input stream.
  long long InputSetsCount() const { return input_sets_count_; }

  // Returns the number of seeks within the candidate list performed
  // by FindAllMaximalSets.
  long long CandidateSeekCount() const { return canidate_seek_count_; }

  // A list of itemsets used to store candidates within the candidate
  // map.
  typedef std::vector<SetProperties*> CandidateList;

 private:
  // First method called by FindAllMaximalSets for rudimentary variable
  // initialization.
  void Init();

  // Prepare datastructures for scanning the data beginning at the
  // provided offset. Returns false if IO error encountered.
  bool PrepareForDataScan(
      DataSourceIterator* data, uint32_t max_item_i, off_t seek_offset);

  // Delete any candidate subsumed by the given input_set.
  void DeleteSubsumedCandidates(unsigned int candidate_index);
  void DeleteSubsumedCandidates(const ItemSet& itemset);

  // Call FoundMaximalSet for all sets that remain as candidates, and
  // release them from memory.  The candidate_ set will be empty
  // upon return.
  void DumpMaximalSets();

  // Invoked for each maximal set found.
  void FoundMaximalSet(const SetProperties& maximal_set);

  // Deletes all candidates from the specified range that are subsumed
  // by the current_set_.
  void DeleteSubsumedFromRange(
    CandidateList::iterator begin_range_it,
    CandidateList::iterator end_range_it,
    const uint32_t* current_set_it,
    unsigned int depth);

  // Invoked by Recurse to delete & advance over any candidates that
  // are equal to the current prefix (and are hence subsumed).
  void DeleteSubsumedSets(
    CandidateList::iterator* begin_range_it,
    CandidateList::iterator end_range_it,
    unsigned int depth);

  CandidateList::iterator GetNewBeginRangeIt(
      CandidateList::iterator begin_range_it,
      CandidateList::iterator end_range_it,
      unsigned int current_item,
      unsigned int depth);

  CandidateList::iterator GetNewEndRangeIt(
      CandidateList::iterator begin_range_it,
      CandidateList::iterator end_range_it,
      unsigned int current_item,
      unsigned int depth);

  // Stats variables.
  long maximal_sets_count_;
  long input_sets_count_;
  long long canidate_seek_count_;

  // Maps each item to a list of "candidate itemsets", each of which
  // contains the item as its first entry.  Itemsets in each candidate
  // list appear in increasing order of cardinality, then increasing
  // lexocographic order. Some entries may be NULL.
  CandidateList candidates_;

  // Index into candidates_. Maps each item id to the position within
  // candidates_ containing the first set in the lexicographic
  // ordering to follow the singleton set { item_id }.
  std::vector<CandidateList::size_type> index_;

  // Temporary/global variables
  SetProperties* current_set_;

  // Configuration options.
  uint32_t items_in_ram_, max_items_in_ram_;
  OutputModeEnum output_mode_;
};

}  // namespace google_extremal_sets

#endif  // _ALL_MAXIMAL_SETS_LEXICOGRAPHIC_H_
