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
// Misc typedefs, enums, and includes required by many files
// in the package.
// ---
// Author: Roberto Bayardo
//
#ifndef _BASIC_TYPES_H_
#define _BASIC_TYPES_H_

#ifdef MICROSOFT   // MS VC++ specific code
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#include <sys/types.h>
#endif

#include <vector>

namespace google_extremal_sets {

typedef std::vector<uint32_t> ItemSet;

enum OutputModeEnum {
  COUNT_ONLY,   // Only count maximal sets. No output to stdout.
  ID,           // Output ids of maximal sets to stdout.
  ID_AND_ITEMS  // Output the id and items from each maximal set to stdout.
};

}  // namespace google_extremal_sets

#endif  // _BASIC_TYPES_H_
