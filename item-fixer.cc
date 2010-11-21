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

#include <algorithm>
#include <ext/hash_map>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "basic-types.h"
#include "data-source-iterator.h"
#include "set-properties.h"

using namespace std;
using __gnu_cxx::hash_map;

namespace google_extremal_sets {

SetPropertiesCompareFunctor compare_set_properties;
SetPropertiesCardinalityCompareFunctor compare_set_properties_cardinality;

bool FixItems(
    DataSourceIterator* data, const char* output_path, bool by_cardinality) {
  FILE* output_file = fopen(output_path, "wb");
  if (!output_file) {
    cerr << "; Could not open output file for writing: "
              << output_path << "\n";
    return false;
  }
  vector<uint32_t> clause;

  // First read in the data & compute the literal frequencies.
  hash_map<uint32_t, uint32_t> literals;
  vector<vector<uint32_t>*> clauses;
  uint32_t vector_id;
  cerr << "; Reading data..." << endl;
  int result;
  while ((result = data->Next(&vector_id, &clause)) == 1) {
    for (int i = 0; i < clause.size(); ++i) {
      ++literals[clause[i]];
    }
    clauses.push_back(new vector<uint32_t>(clause));
  }
  if (result < 0)
    return false;
  cerr << "; Done reading data." << endl;
  
  // Now assign each literal an item id, by replacing its
  // frequency in the literal hash_map with its id.
  {
    uint32_t item_id = 1;
    vector<pair<uint32_t, uint32_t> > frequency_to_item;
    for (hash_map<uint32_t, uint32_t>::const_iterator it = literals.begin();
	 it != literals.end();
	 ++it) {
      frequency_to_item.push_back(make_pair(it->second, it->first));
    }
    sort(frequency_to_item.begin(), frequency_to_item.end());
    for (int i = 0; i < frequency_to_item.size(); ++i) {
      pair<uint32_t, uint32_t>& pair = frequency_to_item[i];
      //cout << "; Mapping literal " << pair.second << " to item_id: " << item_id << '\n';
      literals[pair.second] = item_id;
      ++item_id;
      // TODO: output the literal -> item_id mapping to a file or something.
    }
  }
  
  // Now convert the clauses into Apriori itemsets.
  ItemSet items;
  vector<SetProperties*> sort_us;
  for (int i = 0; i < clauses.size(); ++i) {
    items.clear();
    vector<uint32_t>& clause = *(clauses[i]);
    items.clear();
    for (int j = 0; j < clause.size(); ++j) {
      items.push_back(literals[clause[j]]);
    }
    sort(items.begin(), items.end());
    SetProperties* set_properties = SetProperties::Create(i, items);
    delete clauses[i];
    sort_us.push_back(set_properties);
  }
  clauses.clear();

  // Finally appropriately sort, then write the output.
  cerr << "; Sorting ("
       << (by_cardinality ? "by cardinality" : "lexicographic")
       << ") ..." << endl;
  if (by_cardinality)
    sort(sort_us.begin(), sort_us.end(), compare_set_properties_cardinality);
  else
    sort(sort_us.begin(), sort_us.end(), compare_set_properties);
  cerr << "; Writing " << sort_us.size() << " itemsets to file..." << endl;
  for (uint32_t i = 0; i < sort_us.size(); ++i) {
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
