## Buld requirements

 - CMake >= 3.16
 - A modern C++ compiler supporting C++17 (such as GCC 7+ or Clang 7+)

## Build instructions

```bash
   mkdir build && cd build
   cmake ..
   cmake --build . -j $(nproc)
```

## Usage

```bash
kmer_list_to_hist kmer_list > kmer_list.hist
```
