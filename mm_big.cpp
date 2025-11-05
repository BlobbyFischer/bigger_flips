/***********************************************************************
mm_big.cpp

Based on 2023 Jakob Moosbauer
Modifications Copyright 2025 Isaac Wood

This file is part of bigger_flips.

bigger_flips is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

bigger_flips is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with bigger_flips. If not, see <https://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mm_big.hpp"

MM_big::MM_big(string filename, int n, int m, int l) : Tensor_big(){
  this->n = n;
  this->m = m;
  this->l = l;
  
  maxrank = n*m*l;

  //  cout << "Reading scheme from file " << filename << endl;
  //  cout << "setting maxrank to " << maxrank << endl;
  data = new factor_big[3*maxrank];
  rank = 0;

  bool isLargeFormat = false;
  if(filename.length() > 5 && filename.substr(filename.length() - 5) == ".lexp"){
    isLargeFormat = true;
  }

  string line;
  string a,b,c;
  ifstream input(filename);
  while(getline(input, line)){
    if(line.find('*') == string::npos) continue; // skip lines in files without '*'
    int d1 = line.find('*');
    int d2 = line.find('*',d1+1);
    a = line.substr(0,d1);
    b = line.substr(d1+1, d2-d1-1);
    c = line.substr(d2+1);
    parseMatrix_big(a,'a',m,data+3*rank, isLargeFormat);
    parseMatrix_big(b,'b',l,data+3*rank+1, isLargeFormat);
    parseMatrix_big(c,'c',n,data+3*rank+2, isLargeFormat);
    ++rank;
  }
  input.close();

  flips = new PairSet[3];
  init();
}

MM_big::MM_big(const MM_big &t) : Tensor_big(t){
  n = t.n;
  m = t.m;
  l = t.l;
}

MM_big* MM_big::clone() const {
  return new MM_big(*this);
}


void parseMatrix_big(string s, char x, int m, factor_big* matrix, bool isLargeFormat){
  *matrix = 0;
  int pos = -1;
  if(isLargeFormat){
    //NEW FORMAT - expects a{ii}{jj} e.g a0121 for a_{1,21}
    while((pos = s.find(x,pos+1))!=string::npos){
      setm_big(*matrix,m,std::stoi(s.substr(pos+1,2)),std::stoi(s.substr(pos+3,2)));
    }
  }else{
    while((pos = s.find(x,pos+1))!=string::npos){
      setm_big(*matrix,m,s[pos+1]-'0',s[pos+2]-'0');
    }
  }
}

void MM_big::write(string filename){
  //CHECK IF WE NEED TO WRITE BIG OR SMALL
  bool isLargeFormat = false;
  if(filename.length() > 5 && filename.substr(filename.length() - 5) == ".lexp"){
    isLargeFormat = true;
  }
  ofstream output(filename);
  for(auto r=0; r<rank; ++r){
    writeMatrix_big(output,'a',n,m,get(r,0),isLargeFormat);
    output << '*';
    writeMatrix_big(output,'b',m,l,get(r,1),isLargeFormat);
    output << '*';
    writeMatrix_big(output,'c',l,n,get(r,2),isLargeFormat);
    output << endl;
  }
  output.close();
}

void MM_big::writetoconsole(){
  bool isLargeFormat = (n > 9 || m > 9 || l > 9);
  ostringstream output;
  for(auto r=0; r<rank; ++r){
    writeMatrix_big(output,'a',n,m,get(r,0),isLargeFormat);
    output << '*';
    writeMatrix_big(output,'b',m,l,get(r,1),isLargeFormat);
    output << '*';
    writeMatrix_big(output,'c',l,n,get(r,2),isLargeFormat);
    output << endl;
  }
  output << endl;
  cout << output.str();
}

void writeMatrix_big(ostream &output, char x, int n, int m, factor_big matrix, bool isLargeFormat){
  output << '(';
  if(isLargeFormat){
    for(int i = 1; i <= n; ++i){
      for(int j = 1; j <= m; ++j){
        if(getm_big(matrix, m, i, j)){
          stringstream ss;
          ss << x;
          ss << setfill('0') << setw(2) << i;
          ss << setfill('0') << setw(2) << j;
          output << ss.str() + "+";
        }
      }
    }
  }else{
    for(int i = 1; i <= n; ++i){
      for(int j = 1; j <= m; ++j){
        if(getm_big(matrix, m, i, j)){
          output << x << i << j << '+';
        }
      }
    }
  }
  output.seekp(-1,ios::cur);
  output << ')';
}

//Test whether a scheme is a correct matrix multiplication scheme
bool MM_big::iscorrect(){
  if(correctness_check==0){
    return true;
  }
  factor_big** t = new factor_big*[n*m];
  for(int i = 0; i < n*m; ++i){
    t[i] = new factor_big[m*l];
    for(int j = 0; j < m*l; ++j){
      t[i][j] = 0;
    }
  }
  
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < m; ++j){
      for(int k = 0; k < l; ++k){
	t[m*i+j][l*j+k] = ((factor_big)1) << (n*k+i);
      }
    }
  }
  
  for(int s = 0; s < rank; ++s){
    for(int i = 0; i < n*m; ++i){
      for(int j = 0; j < m*l; ++j){
	if(get(s,0)&((factor_big)1)<<i && get(s,1)&((factor_big)1)<<j){
	  t[i][j] ^= get(s,2);
	}
      }
    }
  }
  
  for(int i = 0; i < n*m; ++i){
    for(int j = 0; j < m*l; ++j){
      if(t[i][j] != 0){
	return false;
      }
    }
    delete[] t[i];
  }
  delete[] t;
  return true;
}

