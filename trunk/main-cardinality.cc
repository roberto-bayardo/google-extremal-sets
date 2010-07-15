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
// limitations under the License.// Copyright 2010 Google Inc.
//
// ---
// Invoke the itemset cardinality-based algorithm for all maximal sets.
// ---
// Author: Roberto Bayardo

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>

#include "all-maximal-sets-cardinality.h"
#include "data-source-iterator.h"

using google_extremal_sets::DataSourceIterator;

int main(int argc, char** argv) {
  time_t start_time;
  time(&start_time);

  // Verify input arguments.
  if (argc != 2) {
    std::cerr << "ERROR: Usage is: ./ams-cardinality <dataset_path>\n";
    return 1;
  }

  {
    std::auto_ptr<DataSourceIterator> data(DataSourceIterator::Get(argv[1]));
    if (!data.get())
      return 2;

    google_extremal_sets::AllMaximalSetsCardinality ap;
    bool result = ap.FindAllMaximalSets(
        data.get(),
        8000000/*max_item_id*/,
        1000000000/*max_items_in_ram*/,
        google_extremal_sets::COUNT_ONLY/*output_mode*/);
    if (!result) {
      std::cerr << "IO ERROR: " << data->GetErrorMessage() << "\n";
      return 3;
    }

    std::cerr << "; Found " << ap.MaximalSetsCount() << " maximal itemsets.\n"
              << "; Number of itemsets in the input: " << ap.InputSetsCount()
              << "\n"
              << "; Number of subsumption checks performed: "
              << ap.SubsumptionChecksCount() << "\n";
  }

  time_t end_time;
  time(&end_time);
  std::cerr << "; Total running time: " << (end_time - start_time)
            << " seconds" << std::endl;

  return 0;
}
