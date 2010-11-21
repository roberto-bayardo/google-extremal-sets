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
// Invoke the item-fixer utility to fix a given a binary formatted dataset.
// To invoke:
//
// ./item-fixer [-c] <path_to_dimacs_dataset> <path_to_output_dataset>
//
// If -c option is specified, the input dataset will be sorted in
// increasing cardinality of its itemsets. Otherwise the dataset will
// be sorted in increasing lexicographic order of its itemsets.
// ---
// Author: Roberto Bayardo

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <memory>

#include "item-fixer.h"
#include "data-source-iterator.h"

using google_extremal_sets::DataSourceIterator;

int main(int argc, char** argv) {
  time_t start_time;
  time(&start_time);

  // Verify input arguments.
  if (argc != 3 && argc != 4 ||
      (argc == 4 && strcmp(argv[1], "-c") != 0)) {
    std::cerr
        << "ERROR: Usage is: ./item-fixer [-c] <input_dataset_path>"
        << " <output_dataset_path>\n";
    return 1;
  }
  bool by_cardinality = (argc == 4);
  int offset = (argc == 4) ? 1 : 0;

  {
    std::auto_ptr<DataSourceIterator> data(
        DataSourceIterator::Get(argv[1 + offset]));
    if (!data.get())
      return 2;
    bool result = google_extremal_sets::FixItems(
        data.get(), argv[2 + offset], by_cardinality);

    if (!result) {
      std::cerr << "IO ERROR: " << data->GetErrorMessage() << "\n";
      return 3;
    }

    std::cerr << "; Success!\n";
  }

  time_t end_time;
  time(&end_time);
  std::cerr << "; Total running time: " << (end_time - start_time)
            << " seconds" << std::endl;

  return 0;
}
