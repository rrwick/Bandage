/*
 * dotplot.cpp
 *
 *  Created on: Oct 16, 2017
 *      Author: Ivan Sovic
 *      GitHub: @isovic
 *      Copyright: Ivan Sovic, 2017
 *      Licence: MIT
 *
 * Simple tool that collects all kmer hits between
 * two sequences. If drawn, this represents a dotplot
 * between two sequences. Can be used for very simple
 * mapping as well.
 */

#include "dotplot.h"

#include <algorithm>
#include <tuple>
#include <sstream>
#include <assert.h>

const int8_t nuc_to_2bit[256] = {
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 0 - 15
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 16 - 31
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 32 - 47
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 48 - 63
  4, 0, 4, 1,   4, 4, 4, 2,   4, 4, 4, 4,   4, 4, 4, 4,  // 64 - 79 (A, C, G)
  4, 4, 4, 4,   3, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 80 - 95 (T)
  4, 0, 4, 1,   4, 4, 4, 2,   4, 4, 4, 4,   4, 4, 4, 4,  // 96 - 111
  4, 4, 4, 4,   3, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 112 - 127
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 128 - 143
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 144 - 159
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 160 - 176
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 176 - 191
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 192 - 208
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 208 - 223
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,  // 224 - 239
  4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4,   4, 4, 4, 4  // 240 - 256
};

const int8_t nuc_to_complement[256] = {
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 0 - 15
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 16 - 31
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 32 - 47
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 48 - 63
  78, 84, 78, 71,   78, 78, 78, 67,   78, 78, 78, 78,   78, 78, 78, 78,  // 64 - 79 (A, C, G)
  78, 78, 78, 78,   65, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 80 - 95 (T)
  78, 84, 78, 71,   78, 78, 78, 67,   78, 78, 78, 78,   78, 78, 78, 78,  // 96 - 111
  78, 78, 78, 78,   65, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 112 - 127
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 128 - 143
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 144 - 159
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 160 - 176
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 176 - 191
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 192 - 208
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 208 - 223
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,  // 224 - 239
  78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78,   78, 78, 78, 78  // 240 - 256
};


std::string reverseComplement(const std::string& seq) {
  std::stringstream ss;
  for (int32_t i = ((int32_t) seq.size()) - 1; i >= 0; i--) {
    ss << nuc_to_complement[(int32_t) seq[i]];
  }
  return ss.str();
}

std::vector<KmerPos> hashKmers(const std::string& seq, int32_t k, bool seq_is_rev) {
  std::vector<KmerPos> ret;

  if (((int32_t) seq.size()) < k) {
    return ret;
  }

  if (k <= 0 || k >= 31) {
    return ret;
  }

  // Pre-scan the sequence and check whether it contains
  // only [ACTG] bases. We do not allow other bases, because
  // they will be packed as 2-bit values in a hash key.
  for (size_t i = 0; i < seq.size(); i++) {
    if (nuc_to_2bit[(int32_t) seq[i]] > 3) {
      return ret;
    }
  }

  int64_t buff = 0x0;
  int64_t buff_mask = (((int64_t) 1) << (2 * k)) - 1; // Clear the upper bits.

  ret.reserve(seq.size() - k + 1);

  // Initialize the buffer.
  for (int32_t i = 0; i < (k - 1); i++) {
    int64_t conv_val = nuc_to_2bit[(int32_t) seq[i]];
    buff = (((int64_t) buff) << 2) | (conv_val & 0x03);
  }

  for (int32_t i = (k - 1); i < (int32_t) seq.size(); i++) {
    // Update the buffer
    int64_t conv_val = nuc_to_2bit[(int32_t) seq[i]];
    buff = (((int64_t) buff) << 2) | (conv_val & 0x03);
    buff &= buff_mask;

    int32_t pos = (seq_is_rev == false) ? (i - k + 1) : (seq.size() - (i - k + 1) - 1);
    ret.emplace_back(KmerPos(buff, pos));
  }

  return ret;
}

std::vector<KmerHit> findHits(const std::vector<KmerPos>& sorted_kmers_seq1, const std::vector<KmerPos>& sorted_kmers_seq2) {
  int32_t k1 = 0, k2 = 0;

  int32_t n_kmers1 = sorted_kmers_seq1.size();
  int32_t n_kmers2 = sorted_kmers_seq2.size();

  std::vector<KmerHit> hits;

  // Check the sortedness of input.
  for (size_t i = 1; i < sorted_kmers_seq1.size(); i++) {
    if(sorted_kmers_seq1[i].kmer < sorted_kmers_seq1[i - 1].kmer) {
      return hits;
    }
  }
  for (size_t i = 1; i < sorted_kmers_seq2.size(); i++) {
    if(sorted_kmers_seq2[i].kmer < sorted_kmers_seq2[i - 1].kmer) {
      return hits;
    }
  }

  while (k1 < n_kmers1 && k2 < n_kmers2) {
    while (k1 < n_kmers1 && sorted_kmers_seq1[k1].kmer < sorted_kmers_seq2[k2].kmer) {
      k1 += 1;
    }
    if (k1 >= n_kmers1) { break; }

    while (k2 < n_kmers2 && sorted_kmers_seq2[k2].kmer < sorted_kmers_seq1[k1].kmer) {
      k2 += 1;
    }
    if (k2 >= n_kmers2) { break; }

    // If the values are not the same, just keep on gliding.
    if (sorted_kmers_seq1[k1].kmer != sorted_kmers_seq2[k2].kmer) {
      continue;
    }

    // Find n^2 exact hits.
    for (int32_t i = k2; i < n_kmers2 && sorted_kmers_seq2[i].kmer == sorted_kmers_seq1[k1].kmer; i++) {
      hits.emplace_back(KmerHit(sorted_kmers_seq1[k1].pos, sorted_kmers_seq2[i].pos));
    }
    k1 += 1;
  }

  return hits;
}

std::vector<KmerHit> findKmerMatches(const std::string& seq1, const std::string& seq2, int32_t k) {
  auto kmers_seq1 = hashKmers(seq1, k, false);
  auto kmers_seq2 = hashKmers(seq2, k, false);
  auto kmers_seq2_rev = hashKmers(reverseComplement(seq2), k, true);

  std::sort(kmers_seq1.begin(), kmers_seq1.end(), [](const KmerPos& a, const KmerPos& b) { return (a.kmer < b.kmer || (a.kmer == b.kmer && a.pos < b.pos)); } );
  std::sort(kmers_seq2.begin(), kmers_seq2.end(), [](const KmerPos& a, const KmerPos& b) { return (a.kmer < b.kmer || (a.kmer == b.kmer && a.pos < b.pos)); } );
  std::sort(kmers_seq2_rev.begin(), kmers_seq2_rev.end(), [](const KmerPos& a, const KmerPos& b) { return (a.kmer < b.kmer || (a.kmer == b.kmer && a.pos < b.pos)); } );

  auto hits = findHits(kmers_seq1, kmers_seq2);
  auto hits_rev = findHits(kmers_seq1, kmers_seq2_rev);

  hits.insert(hits.end(), hits_rev.begin(), hits_rev.end());

  return hits;
}
