ifeq ($(strip $(shell command -v clang++)),)
  CXX := g++
else
  CXX := clang++
endif

all: tensor.cpp tensor.hpp mm.cpp mm.hpp tensor_big.cpp tensor_big.hpp mm_big.cpp mm_big.hpp main_mm.cpp pairSet.cpp pairSet.hpp
	$(CXX) main_mm.cpp tensor.cpp tensor_big.cpp mm.cpp mm_big.cpp pairSet.cpp -O3 -std=c++11
	mv a.out flip
