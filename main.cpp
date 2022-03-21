#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include <filesystem>
#include <fstream>
#include <string>

void make_hist_complex(const std::string &f) {
    absl::flat_hash_map<uint64_t, uint8_t> hist8;
    absl::flat_hash_map<uint64_t, uint16_t> hist16;
    absl::flat_hash_map<uint64_t, uint32_t> hist32;
    absl::flat_hash_map<uint64_t, uint64_t> hist64;
    absl::btree_set<uint64_t> keys;

    std::ifstream in_file;
    std::string line;
    in_file.open(f);

    uint64_t i = 0;
    uint64_t records_processed = 0;
    auto t0 = absl::Now();

    while (std::getline(in_file, line)) {
        const auto freq = ++absl::StrSplit(line, '\t').begin();
        const auto n = static_cast<uint64_t>(std::stoul(freq->data()));
        keys.insert(n);
        if (hist64.contains(n)) {
            ++hist64[n];
        } else if (hist32.contains(n)) {
            if (++hist32[n] == UINT32_MAX) {
                hist32.erase(n);
                if (hist64.contains(n)) {
                    hist64[n] += UINT32_MAX;
                } else {
                    hist64[n] = UINT32_MAX;
                }
            }
        }

        else if (hist16.contains(n)) {
            if (++hist16[n] == UINT16_MAX) {
                hist16.erase(n);
                if (hist32.contains(n)) {
                    hist32[n] += UINT16_MAX;
                } else {
                    hist32[n] = UINT16_MAX;
                }
            }
        }

        else {
            if (++hist8[n] == UINT8_MAX) {
                hist8.erase(n);
                if (hist16.contains(n)) {
                    hist16[n] += UINT8_MAX;
                } else {
                    hist16[n] = UINT8_MAX;
                }
            }
        }
        if (++i == 100'000'000) {
            records_processed += i;
            absl::FPrintF(stderr, "Processed %.1f billion records so far (%.2f records/s)\n", records_processed / 1.0e9, 100.0e6 / absl::ToDoubleSeconds(absl::Now() - t0));
            i = 0;
            t0 = absl::Now();
        }
    }

    in_file.close();

    uint64_t j = 0;
    std::vector<uint64_t> kk{keys.begin(), keys.end()};
    for (const auto k : kk) {
        ++j;
        if (hist64.contains(k)) {
            assert(test_hist[k] == hist64[k]);
            absl::PrintF("%lu\t%lu\n", k, hist64[k]);
        } else if (hist32.contains(k)) {
            assert(test_hist[k] == hist32[k]);
            absl::PrintF("%lu\t%lu\n", k, hist32[k]);
        } else if (hist16.contains(k)) {
            assert(test_hist[k] == hist16[k]);
            absl::PrintF("%lu\t%lu\n", k, hist16[k]);
        } else if (hist8.contains(k)) {
            assert(test_hist[k] == hist8[k]);
            absl::PrintF("%lu\t%lu\n", k, hist8[k]);
        }
    }
}


void make_hist_simple(const std::string &f) {
    absl::btree_map<uint64_t, uint64_t> hist;

    std::ifstream in_file;
    std::string line;
    in_file.open(f);

    uint64_t i = 0;
    uint64_t records_processed = 0;
    auto t0 = absl::Now();

    while (std::getline(in_file, line)) {
        const auto freq = ++absl::StrSplit(line, '\t').begin();
        const auto n = std::stoul(freq->data());
        if (hist.contains(n)) {
            ++hist[n];
        } else {
            hist[n] = 1;
        }
        if (++i == 100'000'000) {
            records_processed += i;
            absl::FPrintF(stderr, "Processed %.1f billion records so far (%.2f records/s)\n", records_processed / 1.0e9, 100.0e6 / absl::ToDoubleSeconds(absl::Now() - t0));
            i = 0;
            t0 = absl::Now();
        }
    }
    in_file.close();

    for (const auto &[k, v] : hist) {
        absl::PrintF("%lu\t%lu\n", k, v);
    }
}

void sync_counts(absl::flat_hash_map<uint64_t, uint64_t> &hist_tmp, absl::btree_map<uint64_t, uint64_t> &hist) {
    absl::FPrintF(stderr, "Writing counts to main counter...");
    const auto t0 = absl::Now();
    uint64_t new_entries = 0;
    for (const auto &[k, v] : hist_tmp) {
        if (hist.contains(k)) {
            hist[k] += v;
        } else {
            hist[k] = v;
            ++new_entries;
        }
    }
    absl::FPrintF(stderr, " DONE in %s. Added %lu new entries and updated %lu.\n", absl::FormatDuration(absl::Now() - t0), new_entries, hist_tmp.size() - new_entries);
    hist_tmp.clear();
}

void make_hist(const std::string &f) {
    absl::btree_map<uint64_t, uint64_t> hist;
    absl::flat_hash_map<uint64_t, uint64_t> hist_tmp;
    uint64_t hist_tmp_capacity = 25'000;
    hist_tmp.reserve(hist_tmp_capacity);

    std::ifstream in_file;
    std::string line;
    in_file.open(f);

    uint64_t i = 0;
    uint64_t records_processed = 0;
    auto t0 = absl::Now();

    while (std::getline(in_file, line)) {
        const auto freq = ++absl::StrSplit(line, '\t').begin();
        const auto n = static_cast<uint64_t>(std::stoul(freq->data()));
        if (hist_tmp.contains(n)) {
            ++hist_tmp[n];
        } else {
            hist_tmp[n] = 1;
        }
        if (hist_tmp.size() >= 0.75 * hist_tmp_capacity) sync_counts(hist_tmp, hist);

        if (++i == 100'000'000) {
            records_processed += i;
            absl::FPrintF(stderr, "Processed %.1f billion records so far (%.2f records/s)\n", records_processed / 1.0e9, 100.0e6 / absl::ToDoubleSeconds(absl::Now() - t0));
            i = 0;
            t0 = absl::Now();
        }
    }
    if (!hist_tmp.empty()) sync_counts(hist_tmp, hist);
    in_file.close();

    for (const auto &[k, v] : hist) {
        absl::PrintF("%lu\t%lu\n", k, v);
    }
}


int main(const int argc, const char **argv) {
    const std::string usage = absl::StrFormat("Incorrect number of arguments!\nUsage: %s kmer_freq.txt [--simple | --complex]", argv[0]);
    if (argc < 2 || argc > 3) {
        absl::FPrintF(stderr, "%s\n", usage);
        return 1;
    }
    auto t0 = absl::Now();
    std::string kmer_freq_list = argv[1];
    std::string mode = argc == 3 ? argv[2] : "";
    if (!std::filesystem::exists(kmer_freq_list)) {
        absl::FPrintF(stderr, "Invalid file '%s'\n", kmer_freq_list);
        return 1;
    }
    if (mode == "--simple") {
        make_hist_simple(kmer_freq_list);
    } else if (mode == "--complex") {
        make_hist_complex(kmer_freq_list);
    } else {
        make_hist(kmer_freq_list);
    }

    absl::FPrintF(stderr, "Histogram calculation took %s.\n", absl::FormatDuration(absl::Now() - t0));

    return 0;
}
