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
// 1) Sets in the file are assumed to appear in increasing order of
// set cardinality. That is, a set of cardinality $i$ will always
// appear before any set of cardinality $j$ > $i$.
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
#ifndef _ALL_MAXIMAL_SETS_CARDINALITY_H_
#define _ALL_MAXIMAL_SETS_CARDINALITY_H_

#include <vector>
#include "basic-types.h"

namespace google_extremal_sets {

class DataSourceIterator;
class SetProperties;

class AllMaximalSetsCardinality {
 public:
  AllMaximalSetsCardinality() {
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
  // The caller must also specify a bound on the number of 4-byte item
  // ids that will be stored in main memory during algorithm
  // execution. Should the dataset contain more items than the limit,
  // the algorithm will switch to an "out of core" mode and perform
  // multiple passes over the data in order to compute the output.
  //
  // This method may output status & progress messages to stderr.
  bool FindAllMaximalSets(
      DataSourceIterator* data,
      uint32_t max_item_id,
      uint32_t max_items_in_ram,
      OutputModeEnum output_mode);

  // Returns the number of maximal sets found by the last call to
  // FindAllMaximalSets.
  long MaximalSetsCount() const { return maximal_sets_count_; }

  // Returns the number of itemsets encountered in the input stream
  // during the lass call to FindAllMaximalSets.
  long long InputSetsCount() const { return input_sets_count_; }

  // Returns the number of explicit subsumption checks performed by
  // the last call to FindAllMaximalSets.
  long long SubsumptionChecksCount() const { return subsumption_checks_count_; }

 private:
  // First method called by FindAllMaximalSets for rudimentary variable
  // initialization.
  void Init();

  // Prepare datastructures for scanning the data beginning at the
  // provided offset. Returns false if IO error encountered.
  bool PrepareForDataScan(
      DataSourceIterator* data, int max_item_i, off_t seek_offset);

  // A list of itemsets used to store candidates within the candidate
  // map.
  typedef std::vector<SetProperties*> CandidateList;

  // Place all sets from index_us into the candidate index.
  void IndexSets(const std::vector<SetProperties*>& index_us);

  // Delete all sets in RAM that are proper subsets of the given set.
  void DeleteSubsumedCandidates(const std::vector<uint32_t>& input_set);

  // Candidate iterator method used by DeleteSubsumedCandidates for
  // each candidate list.
  SetProperties* NextCandidate(
      const CandidateList& candidates,
      const std::vector<uint32_t>& current_set,
      int current_index,
      int* candidate_index);

  // Dump out & delete all sets that remain in the candidate index and
  // those in the list of unindexed_sets.
  void DumpMaximalSets(
      std::vector<SetProperties*>* unindexed_sets,
      OutputModeEnum output_mode);

  // Invoked for each maximal set found.
  void FoundMaximalSet(const SetProperties& maximal_set, OutputModeEnum mode);

  // Stats variables.
  long maximal_sets_count_;
  long input_sets_count_;
  long long subsumption_checks_count_;

  // Maps each item to a list of "candidate itemsets", each of which
  // contains the item as its first entry.  Itemsets in each candidate
  // list appear in increasing order of cardinality. Some entries may
  // be NULL.
  std::vector<CandidateList> candidates_;
};

}  // namespace google_extremal_sets

#endif  // _ALL_MAXIMAL_SETS_CARDINALITY_H_
