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

#include "dimacs-to-apriori.h"
#include "set-properties.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <ext/hash_map>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using __gnu_cxx::hash_map;

namespace {

string ToString(uint32_t l) {
  char buf[30];
  sprintf(buf, "%u", l);
  return string(buf);
}

}  // namespace

namespace google_extremal_sets {

SetPropertiesCompareFunctor compare_set_properties;
SetPropertiesCardinalityCompareFunctor compare_set_properties_cardinality;

/*static*/
DimacsIterator* DimacsIterator::Get(const char* filename) {
  FILE* data = fopen(filename, "r");
  if (!data) {
    cerr << "ERROR: Failed to open input file ("
              << filename << "): " << strerror(errno) << "\n";
    return 0;
  }
  return new DimacsIterator(data);
}

DimacsIterator::DimacsIterator(FILE* data)
  : data_(data) {
}

DimacsIterator::~DimacsIterator() {
  fclose(data_);
  data_ = 0;
}

int DimacsIterator::Next(vector<int>* vec) {
  vec->clear();
  // Now read the literals, until we reach the "0" terminator.
  // If we ever read a "0" right at the beginning of a line it's probably the header
  // or a comment line, so we can just continue.
  int literal;
  int scan_result;
  while ((scan_result = fscanf(data_, "%u", &literal)) != EOF) {
    if (ferror(data_)) {
      error_ = "Dataset read error, ferror code=" + ToString(ferror(data_));
      return -1;
    }
    if (!scan_result) {
      // No conversions, probably a comment or header line.
      if (vec->size()) {
	error_ = "Unexpected non-integer in clause encountered.";
	return -1;
      }
      char c;
      // Skip the current line and continue.
      while ((scan_result = fscanf(data_, "%c", &c)) != EOF && scan_result != 0 && c != '\n') {
      }
      continue;
    }
    if (literal) {
      vec->push_back(literal);
    } else {
      if (vec->size()) {
	return 1;
      } else {
	error_ = "Empty clause encountered.";
	return -1;
      }
    }
  }
  return 0;
}

bool DimacsToApriori(
    DimacsIterator* data,
    const char* output_path,
    bool by_cardinality) {
  FILE* output_file = fopen(output_path, "wb");
  if (!output_file) {
    cerr << "; Could not open output file for writing: "
              << output_path << "\n";
    return false;
  }
  vector<int> clause;

  // First read in the data & compute the literal frequencies.
  hash_map<int, uint32_t> literals;
  vector<vector<int>*> clauses;
  cerr << "; Reading data..." << endl;
  int result;
  while ((result = data->Next(&clause)) == 1) {
    for (int i = 0; i < clause.size(); ++i) {
      ++literals[clause[i]];
    }
    clauses.push_back(new vector<int>(clause));
  }
  if (result < 0)
    return false;
  cerr << "; Done reading data." << endl;
  
  // Now assign each literal an item id, by replacing its
  // frequency in the literal hash_map with its id.
  {
    int item_id = 1;
    vector<pair<uint32_t, int> > frequency_to_item;
    for (hash_map<int, uint32_t>::const_iterator it = literals.begin();
	 it != literals.end();
	 ++it) {
      frequency_to_item.push_back(make_pair(it->second, it->first));
    }
    sort(frequency_to_item.begin(), frequency_to_item.end());
    for (int i = 0; i < frequency_to_item.size(); ++i) {
      pair<uint32_t, int>& pair = frequency_to_item[i];
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
    vector<int>& clause = *(clauses[i]);
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
