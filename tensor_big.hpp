/***********************************************************************
tensor_big.hpp

Based on 2023 Jakob Moosbauer
Modifications Copyright 2025 Isaac Wood

This file is part of bigger_flips.

bigger_flips is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

bigger_flips is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with bigger_flips. If not, see <https://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef tensor_big_hpp___
#define tensor_big_hpp___

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include "pairSet.hpp"
#include <iomanip>
#include <csignal>

#ifdef DEBUG
#define debug(msg) cerr << msg << endl
#else
#define debug(msg) do {} while(0)
#endif

using namespace std;

typedef __uint128_t factor_big;

extern int oldrank;
extern string filename;
extern int correctness_check;
//extern volatile sig_atomic_t termination_flag;

class Tensor_big{
public:
  int rank;
  int maxrank;
  factor_big* data;
  PairSet* flips;

  Tensor_big();
  Tensor_big(const Tensor_big &t);
  
  ~Tensor_big();

  virtual Tensor_big* clone() const;
  
  virtual void write(string filename);
  virtual void writetoconsole();
  
  virtual string newfilename(bool isLargeFormat);
  void writetofile(bool isLargeFormat, int);

  factor_big& get(int, int);
  void remove(int);
  void init();

  bool flip(int col, int row1, int row2, bool reduce_flag = true);
  void split(int col, int row1, int row2);
  bool reduce();
  void remove_zero_rows();

  bool randomflip(mt19937 &gen, uniform_int_distribution<> &coinflip, bool reduce_flag = true);
  
  void randompath(int steps, mt19937 &gen, int split_distance, bool split, bool restart, bool isLargeFormat);
  bool randomsplit(mt19937 &gen, uniform_int_distribution<> &coinflip, uniform_int_distribution<> &d3, int split_distance);

  virtual bool iscorrect();
};

void writelog_big(string, string, int, int);



#endif
