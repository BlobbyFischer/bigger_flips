/***********************************************************************
mm.hpp

Based on 2023 Jakob Moosbauer
Modifications Copyright 2025 Isaac Wood

This file is part of bigger_flips.

bigger_flips is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

bigger_flips is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with bigger_flips. If not, see <https://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef mm_hpp___
#define mm_hpp___

#include "tensor.hpp"

#define getm(matrix,m,i,j) matrix&(((factor)1)<<(m*(i-1)+j-1))
#define setm(matrix,m,i,j) matrix = matrix|(((factor)1)<<(m*(i-1)+j-1))
#define unset(matrix,m,i,j) matrix = matrix&~(((factor)1)<<(m*(i-1)+j-1))

extern int correctness_check;

class MM: public Tensor{
public:
  int n;
  int m;
  int l;

  MM(string filename, int n);
  MM(string filename, int n, int m, int l);
  MM(const MM &t);

  virtual MM* clone() const;

  virtual void write(string filename);
  virtual void writetoconsole();

  virtual bool iscorrect();
};

void parseMatrix(string s, char x, int m, factor* f, bool isLargeFormat);
void writeMatrix(ostream &output, char x, int n, int m, factor f, bool isLargeFormat);

#endif
