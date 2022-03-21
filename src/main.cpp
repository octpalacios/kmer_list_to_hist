// Copyright (C) 2022 Roberto Rossini <roberros@uio.no>
//
// SPDX-License-Identifier: MIT

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <msstl/charconv.hpp>
#include <string>
#pragma GCC diagnostic pop

using u64 = std::uint64_t;
using usize = std::size_t;

namespace utils {

template <class N>
void throw_except_from_errc(std::string_view tok, usize idx, [[maybe_unused]] const N &field,
                            const char *c, std::errc e) {
  static_assert(std::is_arithmetic<N>());
  std::string base_error;
  if (idx != (std::numeric_limits<usize>::max)()) {
    base_error = absl::StrFormat("Unable to convert field %d (\"%s\") to a ", idx, tok);
  } else {
    base_error = absl::StrFormat("Unable to convert field \"%s\" to", tok);
  }
  if (std::is_integral<N>()) {
    if (std::is_unsigned<N>()) {
      base_error += " a positive integral number";
    } else {
      base_error += " an integral number";
    }
  } else {
    base_error += " a real number";
  }
  if (e == std::errc::invalid_argument) {
    if (c != nullptr) {
      throw std::runtime_error(
          absl::StrFormat("%s. Reason: found an invalid character \"%c\"", base_error, *c));
    }
    throw std::runtime_error(absl::StrFormat("%s. Reason: found an invalid character", base_error));
  }
  if (e == std::errc::result_out_of_range) {
    throw std::runtime_error(absl::StrFormat(
        "%s. Reason: number %s is outside the range of representable numbers [%d, %d].", base_error,
        tok, (std::numeric_limits<N>::min)(), (std::numeric_limits<N>::max)()));
  }

  throw std::logic_error(
      absl::StrFormat("%s. If you see this error, report it to the developers on "
                      "GitHub.\n throw_except_from_errc "
                      "called with an invalid std::errc \"%s\". This should not be possible!",
                      base_error, std::make_error_code(e).message()));
}

template <class N>
void parse_numeric_or_throw(std::string_view tok, N &field) {
  auto [ptr, err] = msstl::from_chars(tok.data(), tok.end(), field);
  if (ptr != tok.end() && err != std::errc{}) {
    throw_except_from_errc(tok, (std::numeric_limits<usize>::max)(), field, ptr, err);
  }
}

template <class N>
N parse_numeric_or_throw(std::string_view tok) {
  N field{};
  utils::parse_numeric_or_throw(tok, field);
  return field;
}
}  // namespace utils

void sync_counts(absl::flat_hash_map<u64, u64> &hist_tmp, absl::btree_map<u64, u64> &hist) {
  absl::FPrintF(stderr, "Writing counts to main counter...");
  const auto t0 = absl::Now();
  u64 new_entries = 0;
  for (const auto &[k, v] : hist_tmp) {
    auto [it, inserted] = hist.try_emplace(k, v);
    if (inserted) {
      ++new_entries;
    } else {
      it->second += v;
    }
  }
  absl::FPrintF(stderr, " DONE in %s. Added %lu new entries and updated %lu.\n",
                absl::FormatDuration(absl::Now() - t0), new_entries, hist_tmp.size() - new_entries);
  hist_tmp.clear();
}

auto make_hist(const std::string &f) {
  absl::btree_map<u64, u64> hist;
  absl::flat_hash_map<u64, u64> hist_tmp(25'000);

  std::ifstream in_file(f);
  if (!in_file) {
    throw std::runtime_error(absl::StrFormat("Failed to open file %s!", f));
  }

  u64 i = 0;
  u64 records_processed = 0;

  const auto hist_tmp_capacity_threshold =
      static_cast<usize>(0.75 * static_cast<double>(hist_tmp.capacity()));

  auto t0 = absl::Now();

  for (std::string line; std::getline(in_file, line);) {
    const auto freq = ++absl::StrSplit(line, '\t').begin();
    const auto n = utils::parse_numeric_or_throw<u64>(freq->data());

    auto [it, inserted] = hist_tmp.try_emplace(n, 1);
    if (!inserted) {
      it->second += 1;
    }

    if (hist_tmp.size() >= hist_tmp_capacity_threshold) {
      sync_counts(hist_tmp, hist);
    }

    if (++i == 100'000'000) {
      records_processed += i;
      absl::FPrintF(stderr, "Processed %.1f billion records so far (%.2f records/s)\n",
                    static_cast<double>(records_processed) / 1.0e9,
                    100.0e6 / absl::ToDoubleSeconds(absl::Now() - t0));
      i = 0;
      t0 = absl::Now();
    }
  }

  in_file.close();

  if (!hist_tmp.empty()) {
    sync_counts(hist_tmp, hist);
  }

  return hist;
}

int main(const int argc, const char **argv) {
  const auto usage = absl::StrFormat("Usage: %s kmer_freq.txt\n", argv[0]);
  if (argc != 2) {
    absl::FPrintF(stderr, "Incorrect number of arguments!\n%s", usage);
    return 1;
  }

  if (std::string_view tok = argv[1]; tok == "--help" || tok == "-h" || tok == "help") {
    absl::FPrintF(stderr, "%s", usage);
    return 0;
  }

  const std::string kmer_freq_list = argv[1];
  if (!std::filesystem::exists(kmer_freq_list)) {
    absl::FPrintF(stderr, "Invalid file '%s'\n", kmer_freq_list);
    return 1;
  }

  auto t0 = absl::Now();
  try {
    const auto histogram = make_hist(kmer_freq_list);
    for (const auto &[k, v] : histogram) {
      absl::PrintF("%lu\t%lu\n", k, v);
    }
  } catch (const std::exception &e) {
    absl::FPrintF(stderr, "The following error occurred while building kmer histogram: %s\n",
                  e.what());
    return 1;
  }

  absl::FPrintF(stderr, "Histogram calculation took %s.\n", absl::FormatDuration(absl::Now() - t0));

  return 0;
}
