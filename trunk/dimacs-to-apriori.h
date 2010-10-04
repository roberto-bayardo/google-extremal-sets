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
// Utility for converting DIMACS formatted SAT instances into apriori
// binary format data.
// ---
// Author: Roberto Bayardo

#ifndef _DIMACS_TO_APRIORI_H_
#define _DIMACS_TO_APRIORI_H_

#include <stdio.h>
#include <string>
#include <vector>
#include "basic-types.h"

namespace google_extremal_sets {

class DimacsIterator {
 public:
  // Factory method for obtaining an iterator for DIMACS formatted
  // propositional satisfiability instances. The filepath is the
  // pathname to the file containing the data. Returns NULL on error
  // and reports the error details to stderr.
  static DimacsIterator* Get(const char* filepath);
  ~DimacsIterator();

  // Returns a human-readable string describing any error condition
  // encountered during a call to Next()/NextText().
  std::string GetErrorMessage() { return error_; }

  // Reads the next input itemset from an "apriori binary" formatted
  // input file.  Returns -1 on error, 0 on EOF, and 1 on
  // success. Each itemset consists of a 4 byte integer ID, a 4 byte
  // integer length, and then 4 byte integer IDs for each item. Checks
  // for many dataset format errors, but not all of them. For example
  // it does not check that the items are duplicate free and are
  // consistently ordered according to frequency.
  int Next(std::vector<int>* literals);

 private:
  DimacsIterator(FILE* data);

  FILE* data_;
  std::string error_;
};  // class DimacsIterator

// Accepts a DimacsIterator and converts the instance into an apriori
// binary formatted dataset, sorted either by_cardinality or
// lexicographically depending on the input parameter.
bool DimacsToApriori(
    DimacsIterator* data,
    const char* output_path,
    bool by_cardinality);

}  // namespace google_extremal_sets

#endif  // _DIMACS_TO_APRIORI_H_
