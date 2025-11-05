/***********************************************************************
main_mm.cpp

Copyright 2023 Jakob Moosbauer
 **********************************************************************/

# include "mm.hpp"
# include "mm_big.hpp"

int oldrank;
string filename;
int correctness_check = 1;

int main(int argc, char* argv[]){
  debug("debugging enabled");

  // Reading command line arguments and setting parameters
  if(argc < 8 || argc > 11){
    cerr << "Wrong number of arguments. Usage: " << endl;
    cerr << argv[0] << " <filename> <dim 1> <dim 2> <dim 3> <path length> <split> <restart> [split distance] [correctness check] [seed]" << endl;
    return 1;
  }
  
  filename = argv[1];
  int l = strtol(argv[2], NULL, 10);
  int m = strtol(argv[3], NULL, 10);
  int n = strtol(argv[4], NULL, 10);

  bool isBig = (l*m > 64 || m*n > 64 || n*l > 64);
  bool isTooBig = (l*m >128 || m*n > 128 || n*l > 128);

  if(isTooBig){
    cerr << "Too big, all matrices must have dimension product most 128." << endl;
    return 1;
  }

  int pathlength = strtol(argv[5],NULL,10);
  bool split = strtol(argv[6],NULL,10);
  bool restart = strtol(argv[7],NULL,10);
  int split_distance = 10;
  int seed = -1;
  
  if(argc >= 9){
    split_distance = strtol(argv[8],NULL,10);
  }
  
  if(argc >= 10){
    correctness_check = strtol(argv[9],NULL,10);
  }
  
  if(argc >= 11){
    seed = strtol(argv[10],NULL,10);
  }
  

  // Reading the scheme

  if(isBig){
    MM_big s = MM_big(filename,n,m,l);

    oldrank = s.rank;

    if(!s.iscorrect()){
      cerr << "Opened incorrect scheme: " << filename << endl;
      return 1;
    }


    // Setting up random number generator

    mt19937 gen;
    if(seed == -1){
      random_device rd;
      gen.seed(rd());
    }
    else{
      gen.seed(seed);
    }

    bool isLargeFormat = (n>9 || m>9 || l>9);

    // Main call

    s.randompath(pathlength, gen, split_distance, split, restart, isLargeFormat);
  }else{
    MM s = MM(filename,n,m,l);

    oldrank = s.rank;

    if(!s.iscorrect()){
      cerr << "Opened incorrect scheme: " << filename << endl;
      return 1;
    }


    // Setting up random number generator

    mt19937 gen;
    if(seed == -1){
      random_device rd;
      gen.seed(rd());
    }
    else{
      gen.seed(seed);
    }

    bool isLargeFormat = (n>9 || m>9 || l>9);

    // Main call

    s.randompath(pathlength, gen, split_distance, split, restart, isLargeFormat);
  }

  return 0;
}
