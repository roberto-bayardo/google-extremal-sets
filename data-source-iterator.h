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
// Iterator for scanning binary or text format datasets.
// ---
// Author: Roberto Bayardo

#ifndef _DATA_SOURCE_ITERATOR_H_
#define _DATA_SOURCE_ITERATOR_H_

#include <stdio.h>
#include <string>
#include "basic-types.h"

namespace google_extremal_sets {

class DataSourceIterator {
 public:
  // Factory method for obtaining an iterator. The filepath is the
  // pathname to the file containing the data. Returns NULL on error
  // and reports the error details to stderr.
  static DataSourceIterator* Get(const char* filepath);
  ~DataSourceIterator();

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
  int Next(uint32_t* vector_id_, ItemSet* input_vector);

  // Like Next, but used when testing with text format files.  Text
  // format assumes whitespace separators between vector and item
  // IDs. Instead of encoding vector lengths, use item id "0" to
  // terminate vectors.  E.g.:
  //
  // 1 1 2 3 0
  // 2 1 2 3 4 0
  // 3 2 3 0
  // ...
  //
  // The first value for a vector is its ID. The remaining values are
  // the IDs of its elements. End of line chars are encouraged, but
  // are not required to separate the vectors.
  int NextText(uint32_t* vector_id_, ItemSet* input_vector);

  bool Seek(off_t seek_offset);
  off_t Tell();

 private:
  DataSourceIterator(FILE* data);

  FILE* data_;
  int lines_processed_;
  std::string error_;
};

}  // google_extremal_sets

#endif  // _DATA_SOURCE_ITERATOR_H_
