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
build/kmer_list_to_hist <(zcat kmer_list.gz) > kmer_list.hist
```

`kmer_list.gz` is a gzipped file like:

```
GGGGGGGGGGAGGGGGGGGGG	33238
GTACACGTGGTGGTGTCAGGT	26
TTTATCGAGCTCAAGTGTGCT	1
GAATGCTGAAGGTTTTACAGT	17
GCAGATTCTGGAGAAATAAAT	1
CTGCGCTGGCGGGCTGCAGAG	1
TGCGATTACAACCCCGAACAG	4
TTGATTTTGAGAAATTTGTTT	1
GGACTGAGCGGTAAAGCTCTG	1
GGTATATAATTTGGGAAATAG	1
...
```
