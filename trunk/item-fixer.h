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
// Utility for imposing proper item ordering on a binary dataset.
// ---
// Author: Roberto Bayardo
//
#ifndef _SORTER_H_
#define _SORTER_H_

namespace google_extremal_sets {

class DataSourceIterator;

// Imposes the proper frequency based item ordering on the dataset.
// Also sorts the input data and writes it to the output_file in
// apriori binary format. Returns false on IO error. Sort order is
// increasing lexicographic if by_cardinality is false, and increasing
// cardinality otherwise.
bool FixItems(
    DataSourceIterator* data,
    const char* output_path,
    bool by_cardinality);

}  // namespace util

#endif  //  _ALL_MAXIMAL_SETS_H_
