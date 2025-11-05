# Flip graphs for fast rank decompositions of matrix multiplication tensors
This project is part of ongoing research by Jakob Moosbauer, Manuel Kauers, Isaac Wood and others into the flip graph approach to finding low rank decompositons of matrix multiplication tensors over F_2.

This repository contains a modified version of existing code to allow for larger tensors.
## Authorship and acknowledgement
This code is almost entirely the work of **Dr Jakob Moosbauer** (Copyright 2023) for https://arxiv.org/abs/2212.01175. This repository simply allows for larger some matrix sizes, as research in this area has managed to match best known bounds or improved them all the way up to (8,8,8).

- **Original author:** Jakob Moosbauer
- **Original project:** https://github.com/jakobmoosbauer/flips
- **Modifications by:** Isaac Wood

## Key features of this version
This fork extends the original in the following ways:
- **Large file format:** The original program read and wrote files in format \*.exp (of which many examples of can be found at https://github.com/jakobmoosbauer/flips/tree/main/222flipgraph_rank8/vertices) which limits the program to matrix multiplication tensors with no dimension greater than 9. We now also include support for a new file type *.lexp which assumes 2 indices for the i,j,k and examples can be found in this repo in the solutions folder.
- **128 bit support:** The program now automatically decides if 64 bits will be sufficient, and if not it uses a new class using 128 bit integers. If it would need more than 128 bits the program will throw an error.

## Getting started

### 1. Dependencies
You will need a C++ compiler that supports the ``__uint128_t`` type and C++11. The makefile provided supports g++ and clang.

### 2. Compiling
A `makefile` is provided, so compiling should be as simple as
```bash
make
```
This should compile the program and create the executable 'flip'.

### 3. Running a search
We can run the program from the command line using
```bash
./flip <filename> <l> <m> <n> <pathlength> <split> <restart> [split_distance] [correctness_check] [seed]
```

**Example**
We can look at the example `222.exp` in the repo; this is the standard <2,2,2> matrix multiplication tensor. Running ```bash ./flip 222.exp 2 2 2 1000000 0 0``` should show something like ```bash k00000000000120e.exp,7```. This means it has found a rank 7 decomposition and saved it in `k00000000000120e.exp`.

#### Argument reference
| Argument | Description |
| :--- | :--- |
| **`<filename>`** | The input file, expected as either a `*.exp` or `*.lexp`, containing the initial tensor decomposition. |
| **`<l>`**, **`<m>`**, **`<n>`** | These are the dimensions of matrix multiplications. These will help the program decide whether it needs to run the larger (and slightly slower) version or if it can run the faster version for smaller tensors. If these are inputted too small for the input tensor the program will crash. |
| **`<pathlength>`** | The number of random flips to do before stopping. |
| **`<split>`** | This will be interpreted as an integer, but used as a boolean (e.g '0' for false, '1' for true, '3' for true). This will tell the program whether you want to perform splits (`true`) or only flips and reductions(`false`). |
| **`<restart>`** | This will be interpreted as an integer, but used as a boolean (e.g '0' for false, '1' for true, '3' for true). This will tell the program whether you want to find just one reduction (`true`) or if you want the program to restart once a reduction has been found to go as far as possible (`false`). Why you may want this set to `true` is explained later in **harder searches** |
| **`[split_distance]`** | *Optional.* The number of flips to do after a split to avoid a trivial reduction. This is set to 10 by default. |
| **`[correctness_check]`** | *Optional.* This will be interpreted as an integer, but used as a boolean (e.g '0' for false, '1' for true, '3' for true). This will tell the program whether to check for correctness (`true`) or not (`false`). You may not want to check for correctness for testing tensors that are not matrix multiplication tensors. |
| **`[seed]`** | *Optional.* This provides a seed to the random number generator for reproducible results. If none is given, then a seed is randomly generated using mt19937.

## Running bigger searches
More often than not in research, we are not looking for a specific tensor, but are using this method to find low rank decompositions of many different tensors, and due to the flip graph search method's stochastic nature, we aim to do as wide of a search as possible. The specifics of this search process (described as creating "pools") are detailed in the original paper https://arxiv.org/abs/2212.01175. This is implemented in "down.py".

### 1. Dependencies
This has been made to run on Python 3.12 using threading, Path, glob, time, random, subprocess, shutil, sys and os.

### 2. The solutions directory structure
Inside the solutions directory is where all the schemes found with `down.py` will be moved, and where `down.py` expects files to be. These are expected in a specific layout. Here is the layout in the solutions directory as provided.
```text
.
├── 2,3,3
│   └── x18
│       └── 233.exp
├── 3,3,3
│   └── x27
│       └── 333.exp
│   └── y23
│       ├── k0000001b8b426de.exp
│       ├── k0000001c70e1b0e.exp
│       ...
├── 4,4,4
│   └── x64
│       └── 444.exp
```

Let's break down each part of this:
**matrix multiplication dimensions:** We have a directory for each `l`, `m` and `n` we wish to look at `<l,m,n>` for.
**prefixes:** We have a prefix to distinguish different methods. Sometimes you may not want to keep these separate, but for documenting the research process it is useful to have it known which approaches did what. We will see an example of this.
**ranks:** We have a directory for each rank for each prefix. This keeps all our schemes nicely organised.
**schemes:** Inside the {prefix}{rank} directories are all the schemes that make up the "pool" for that rank and prefix.

### 3. Using down.py
We can run this from the command line using
```bash
python3 down.py <l> <m> <n> <prefix> [pathlength] [failed_reductions_needed] [reductions_needed] [processors] [splits] [split_distance]
```

**Examples**
- Run this on the 233 example given: ```bash python3 down.py 2 3 3 x```. This should find rank 15.
- Try this for <3,3,3> using ```bash python3 down.py 3 3 3 x```. This should find rank 23.
- Try this for <4,4,4> using ```bash python3 down.py 4 4 4 x```. The rank over F_2 is at most 47, but your computer likely only found 55 or so. Perhaps it will work better using a longer path, or trying more times, but instead we can use a different approach.

#### Argument reference
| Argument | Description |
| :--- | :--- |
| **`<l>`**, **`<m>`**, **`<n>`** | The dimensions of the matrix multiplication tensor you aim to find the rank of. |
| **`<prefix>`** | The prefix of the search method you are using. In the examples given `x` corresponds to starting from the standard decomposition, and `y` corresponds to the "edge transition" approach laid out by Arai et al in https://arxiv.org/abs/2312.16960v1. |
| **`[pathlength]`** | *Optional.* This is the length of the path the program should run a search for. Set to 10000000 by default. |
| **`[failed_reductions_needed]`** | *Optional.* This is the number of times the program needs to fail at finding a reduction before the program decides we have approximated the rank as best we can with this approach. Set to 100 by default; a value of around 1000 was used in https://arxiv.org/abs/2510.19787. |
| **`[reductions_needed]`** | *Optional.* This is the size of the pool of reductions the program aims to find. Set to 25 by default; a value of around 250 was used in https://arxiv.org/abs/2510.19787. |
| **`[processors]`** | *Optional.* This is the number of threads the program should use. Set to 8 by default; a value of around 200 was used in https://arxiv.org/abs/2510.19787. |
| **`[splits]`** | *Optional.* This will be interpreted as an integer, but used as a boolean (e.g '0' for false, '1' for true, '3' for true). This will tell the program whether you want to perform splits (`true`) or only flips and reductions(`false`). Set to '1' by default. |
| **`[split_distance]`** | *Optional.* The number of flips to do after a split to avoid a trivial reduction. This is set to 1 by default. |

Note also the correctness check is turned off in down.py to save time.

### 4. Using expand.py
This is a program for extending a scheme as first described by Arai et al as "edge transitions" in https://arxiv.org/abs/2312.16960v1.
We can run this from the command line using
```bash
python3 expand.py <oldl> <oldm> <oldm> <l> <m> <n> <num_threads>
```
This will expand all files with best rank found for <oldl,oldm,oldn> by method `y` in the solutions folder to <l,m,n> schemes (and save these in the solutions folder).

**Example**
As seen, starting from the standard algorithm is unlikely to find <4,4,4> rank 47. We will now use method `y`, which usually works with the default parameters.
We use the <3,3,3> rank 23 folder provided, though of course in practice you can find this using down.py.
Now we run
```bash
python3 expand.py 3 3 3 3 3 4 8
```
to expand all of our <3,3,3> schemes to <3,3,4> using 8 threads. Now we can reduce these using
```bash
python3 down.py 3 3 4 y
```
And this should find rank 29. Now we continue with
```bash
python3 expand.py 3 3 4 3 4 4 8
python3 down.py 3 4 4 y
python3 expand.py 3 4 4 4 4 4 8
python3 down.py 4 4 4 y
```
And this should find <4,4,4> of rank 47. The whole process will likely take a few minutes, but if it gets stuck at a bottleneck it could take around 10 minutes. Note also that this process is stochastic and so this example working as described is down partially to luck. To increase your chances of it working, increase the `failed_reductions_needed` parameter (to avoid missing bottlenecks). For larger searches, you will likely want to increase all search parameters.

#### Argument reference
| Argument | Description |
| :--- | :--- |
| **`<oldl>`**, **`<oldm>`**, **`<oldn>`** | The dimension of the matrix multiplication tensor decompositions to expand from. |
| **`<l>`**, **`<m>`**, **`<n>`** | The dimension of the matrix multiplication tensor you wish to expand to. |
| **`[num_threads]`** | *Optional.* The number of threads to use to complete this process. Set to 1 by default. |

# License
This project is licensed under the GNU General Public License v3.0. #The C++ code is a derivative work of Jakob Moosbauer's original GPLv3-licensed project. All new contributions (including the Python utility scripts) are also released under the GPLv3.

See the `LICENSE` file for the full text.
