/***********************************************************************
pairSet.hpp

Copyright 2023 Jakob Moosbauer

This file is part of bigger_flips.

bigger_flips is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

bigger_flips is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with bigger_flips. If not, see <https://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef pairset_hpp__
#define pairset_hpp__

#include<vector>
#include<cstdint>

using namespace std;

class PairSet{
  public:
  vector<uint64_t> pairs;

  void insert(uint64_t first, uint64_t second);
  void remove(uint64_t first, uint64_t second);
  void remove(uint64_t elem);
  size_t size();
  void clear();
  uint64_t first(size_t i);
  uint64_t second(size_t i);

  bool contains(uint64_t first, uint64_t second);
  bool contains(uint64_t pair);
};
  
#endif
