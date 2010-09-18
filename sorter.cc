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
// Author: Roberto Bayardo

#include "sorter.h"

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "basic-types.h"
#include "data-source-iterator.h"
#include "set-properties.h"

namespace google_extremal_sets {

SetPropertiesCompareFunctor compare_set_properties;
SetPropertiesCardinalityCompareFunctor compare_set_properties_cardinality;

bool Sort(
    DataSourceIterator* data, const char* output_path, bool by_cardinality) {
  FILE* output_file = fopen(output_path, "wb");
  if (!output_file) {
    std::cerr << "; Could not open output file for writing: "
              << output_path << "\n";
    return false;
  }
  uint32_t set_id;
  std::vector<uint32_t> itemset;
  std::vector<SetProperties*> sort_us;

  int result;
  std::cerr << "; Reading data..." << std::endl;
  while ((result = data->Next(&set_id, &itemset)) == 1) {
    SetProperties* set = SetProperties::Create(set_id, itemset);
    // Check for itemsets that are not properly sorted / contain
    // duplicate items.
    bool not_sorted = false;
    for (int i = 0; i < set->size - 1; ++i) {
      if (set->item[i] >= set->item[i + 1]) {
        not_sorted = true;
        break;
      }
    }
    if (not_sorted) {
      std::cerr << "; WARNING: Skipping invalid set. " << *set << '\n';
      SetProperties::Delete(set);
    } else {
      sort_us.push_back(SetProperties::Create(set_id, itemset));
    }
  }
  if (result < 0)
    return false;
  std::cerr << "; Sorting ("
	    << (by_cardinality ? "by cardinality" : "lexicographic")
	    << ") ..." << std::endl;
  if (by_cardinality)
    sort(sort_us.begin(), sort_us.end(), compare_set_properties_cardinality);
  else
    sort(sort_us.begin(), sort_us.end(), compare_set_properties);
  std::cerr
      << "; Writing " << sort_us.size() << " itemsets to file..." << std::endl;
  for (int i = 0; i < sort_us.size(); ++i) {
    SetProperties* set = sort_us[i];
    if (!fwrite(set, sizeof(uint32_t), 2 + set->size, output_file)) {
      // TODO: fix mem leak
      return false;
    }
    SetProperties::Delete(set);
  }
  if (fclose(output_file))
    return false;
  return true;
}

}  // namespace google_extremal_sets
