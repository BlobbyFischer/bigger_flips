/***********************************************************************
tensor.cpp

Based on 2023 Jakob Moosbauer
Modifications Copyright 2025 Isaac Wood

This file is part of bigger_flips.

bigger_flips is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

bigger_flips is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with bigger_flips. If not, see <https://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tensor.hpp"

namespace{
  static const int plus1mod3[] = {1,2,0};
  static const int plus2mod3[] = {2,0,1};
}

Tensor::Tensor() {
  rank = 0;
  data = NULL;
  flips = NULL;
}

Tensor::~Tensor(){
  delete[] data;
  delete[] flips;
}

Tensor::Tensor(const Tensor &t) {
  rank = t.rank;
  data = new factor[3*rank];
  for(int i = 0; i<3*rank; ++i){
    data[i] = t.data[i];
  }
}

Tensor* Tensor::clone() const {
  return new Tensor(*this);
}

void Tensor::write(string filename){
  cerr << "write method for generic tensor object not implemented" << endl;
}


void Tensor::writetoconsole(){
  cerr << "write method for generic tensor object not implemented" << endl;
}

factor& Tensor::get(int row, int col){
  return data[row*3+col];
}

bool Tensor::reduce(){
  for(auto i = 0; i < flips[0].size(); ++i){
    int r1 = flips[0].first(i);
    int r2 = flips[0].second(i);
    if(get(r1,1) == get(r2,1)){
      get(r1,2) ^= get(r2,2);
      remove(r2);
      return true;
    }
    if(get(r1,2) == get(r2,2)){
      get(r1,1) ^= get(r2,1);
      remove(r2);
      return true;
    }
  }
  for(auto i = 0; i < flips[1].size(); ++i){
    int r1 = flips[1].first(i);
    int r2 = flips[1].second(i);
    if(get(r1,2) == get(r2,2)){
      get(r1,0) ^= get(r2,0);
      remove(r2);
      return true;
    }
  }
  return false;
}

void Tensor::remove(int row){
  data[3*row]=data[3*(rank-1)];
  data[3*row+1]=data[3*(rank-1)+1];
  data[3*row+2]=data[3*(rank-1)+2];
  --rank;
  init();
}

void Tensor::remove_zero_rows(){
  for(int i = 0; i < rank; ++i){
    if(get(i,0) == 0 || get(i,1) == 0 || get(i,2) == 0){
      remove(i);
      --i;
    }
  }
}

bool Tensor::flip(int col, int r1, int r2, bool reduce_flag){
  int a = col;
  int b = plus1mod3[a];
  int c = plus2mod3[a];
  get(r1,c) ^= get(r2,c);
  get(r2,b) ^= get(r1,b);
  flips[c].remove(r1);
  flips[b].remove(r2);
  for(auto i = 0; i < rank; ++i){
    if(i != r1 && get(i,c) == get(r1,c)){
      flips[c].insert(r1,i);
      if(reduce_flag){
	if(get(i,a) == get(r1,a)){
	  reduce();
	  return 1;
	}
	if(get(i,b) == get(r1,b)){
	  reduce();
	  return 1;
	}
      }
    }
    if(i != r2 && get(i,b) == get(r2,b)){
      flips[b].insert(r2,i);
      if(reduce_flag){
	if(get(i,a) == get(r2,a)){
	  reduce();
	  return 1;
	}
	if(get(i,c) == get(r2,c)){
	  reduce();
	  return 1;
	}
      }
    }
  }
  return 0;
}

void Tensor::split(int col, int row1, int row2) {
  if (rank >= maxrank) {
    return;
  }
  int a = col;
  int b = plus1mod3[col];
  int c = plus2mod3[col];
  get(rank,a) = get(row1,a)^get(row2,a);
  get(row1,a) = get(row2,a);
  get(rank,b) = get(row1,b);
  get(rank,c) = get(row1,c);
  rank++;
  flips[a].remove(row1);
  for(auto i=0; i < rank-1; ++i) {
    if(i != row1 && get(i,a) == get(row1,a)) {
      flips[a].insert(i,row1);
    }
    if(get(i,a) == get(rank - 1,a)) {
      flips[a].insert(i,rank - 1);
    }
    if(get(i,b) == get(rank - 1,b)) {
      flips[b].insert(i,rank - 1);
    }
    if(get(i,c) == get(rank - 1,c)) {
      flips[c].insert(i,rank - 1); 
    }
  }
}

string Tensor::newfilename(bool isLargeFormat){
  factor s = 0;
  for(auto i = 0; i < rank; ++i){
    s+=get(i,0)+get(i,1)+get(i,2);
    s<<=1;
    s%=9223372036854775807;
  }
  stringstream stream;
  if(isLargeFormat){
    stream << "k" << setfill('0') << setw(15) << hex << s << ".lexp";
  }else{
    stream << "k" << setfill('0') << setw(15) << hex << s << ".exp";
  }
  string name = stream.str();
  return name;
}

void writelog(string oldfilename, string newfilename, int steps, int oldrank, int rank){
  if (true)
    return; // mk 2024-04-07
  ofstream log;
  log.open(newfilename.substr(0,3) + ".log", ios_base::app);
  log << time(NULL) << "," << oldfilename << "," << oldrank << "," << newfilename << "," << steps << "," << rank << endl;
  log.close();
}

bool Tensor::iscorrect(){
  return true;
}

void Tensor::init(){
  for(int k = 0; k < 3; ++k){
    flips[k].clear();
    for(int i = 0; i<rank; ++i){
      for(int j = i+1; j<rank; ++j){
	if(get(i,k) == get(j,k)){
	  flips[k].insert(i,j);
	}
      }
    }
  }
}

void Tensor::writetofile(bool isLargeFormat, int steps = -1){
  string outputfilename = newfilename(isLargeFormat);
  write(outputfilename);
  cout << outputfilename << "," << rank << endl;
  writelog(filename, outputfilename, steps, oldrank, rank);
}

void Tensor::randompath(int steps, mt19937 &gen, int split_distance, bool split, bool restart, bool isLargeFormat){
  int init_rank = rank;
  uniform_int_distribution<> coinflip(0, 1);
  uniform_int_distribution<> d3(0, 2);
  if(split){
    while(!randomsplit(gen, coinflip, d3, split_distance));
  }
  do{
    int i = 0;
    for(i = 0; i<steps; ++i) {
      int size = flips[0].size() + flips[1].size() + flips[2].size();
      if (size == 0) {
        writetofile(isLargeFormat, i);
        return;
      }
      if (randomflip(gen, coinflip, true)) {
        break;
      }
    }
    if (i == steps) {
      writetofile(isLargeFormat, steps);
      return;
    }
  } while(restart || rank >= init_rank); // don't end because you reduced from a split! If you split, you need to reduce *AGAIN*
  writetofile(isLargeFormat, steps);
  return;
}

bool Tensor::randomflip(mt19937 &gen, uniform_int_distribution<> &coinflip, bool reduce_flag){
  int size = flips[0].size() + flips[1].size() + flips[2].size();
  uniform_int_distribution<> distribution(0, size - 1);
  int r = distribution(gen);
  int row1, row2, col;
  if (r < flips[0].size()) {
    col = 0;
    row1 = flips[0].first(r);
    row2 = flips[0].second(r);
  } else if (r < flips[0].size() + flips[1].size()) {
    col = 1;
    row1 = flips[1].first(r - flips[0].size());
    row2 = flips[1].second(r - flips[0].size());
  } else {
    col = 2;
    row1 = flips[2].first(r - flips[0].size() - flips[1].size());
    row2 = flips[2].second(r - flips[0].size() - flips[1].size());
  }
  if (coinflip(gen)) {
    return flip(col, row1, row2, reduce_flag);
  } else {
    return flip(col, row2, row1, reduce_flag);
  }
}

bool Tensor::randomsplit(mt19937 &gen, uniform_int_distribution<> &coinflip, uniform_int_distribution<> &d3, int split_distance){
  if(rank >= maxrank){
    return true;
    cout << "maxrank reached" << endl;
  }
  uniform_int_distribution<> row_distribution(0, rank - 1);
  int row1, row2;
  do{
    row1 = row_distribution(gen);
    row2 = row_distribution(gen);
  }
  while(row1 == row2);
  int col = d3(gen);
  split(col,row1,row2);
  if(coinflip(gen)){
    flip(col, row1, row2, 0);
  } else {
    flip(col, row2, row1, 0);
  }
  for(int j = 0; j < split_distance; ++j){
    randomflip(gen, coinflip, false);
  }
  int previousrank = rank;
  remove_zero_rows();
  while(reduce());
  if(rank < previousrank){
    return false;
  }
  //  cout << "split successful" << endl;
  return true;
}
