## Dependencies

 - CMake >= 3.12
 - A modern C++ compiler (C++17, GCC 7+, Clang 5+)

## Build instructions

```bash
   git clone --recurse-submodules --remote-submodules https://github.com/robymetallo/kmer_list_to_hist.git
   mkdir build && cd build
   cmake ..
   make -j $(nproc)
```

## Usage

`kmer_list_to_hist kmer_list > kmer_list.hist`

## Notes

The binaries produced by default are not portable.
If you are planning to use this program on multiple machines with different architectures, you may want to edit `CMakeLists.txt` and change `-mtune` and `-march` from `native` to `generic`, and repeat the steps outlined in [build instructions](https://github.com/robymetallo/kmer_list_to_hist#build-instructions).

