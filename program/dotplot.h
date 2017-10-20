/*
 * dotplot.h
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

#ifndef SRC_PROGRAM_DOTPLOT_H_
#define SRC_PROGRAM_DOTPLOT_H_

#include <string>
#include <vector>
#include <stdint.h>

class KmerPos {
 public:
  KmerPos() : kmer(0), pos(0) {}
  KmerPos(int64_t _kmer, int32_t _pos) : kmer(_kmer), pos(_pos) { }
  bool operator==(const KmerPos& b) const {
    return (this->kmer == b.kmer && this->pos == b.pos);
  }
  bool operator<(const KmerPos& b) const {
    return (this->kmer < b.kmer || (this->kmer == b.kmer && this->pos < b.pos));
  }

  int64_t kmer;
  int32_t pos;
};

struct KmerHit {
  KmerHit() : x(0), y(0) { }
  KmerHit(int32_t _x, int32_t _y) : x(_x), y(_y) { }
  bool operator==(const KmerHit& b) const {
    return (this->x == b.x && this->y == b.y);
  }
  bool operator<(const KmerHit& b) const {
    return (this->x < b.x || (this->x == b.x && this->y < b.y));
  }

  int32_t x;
  int32_t y;
};

std::string reverseComplement(const std::string& seq);
std::vector<KmerPos> hashKmers(const std::string& seq, int32_t k, bool seq_is_rev);
std::vector<KmerHit> findHits(const std::vector<KmerPos>& sorted_kmers_seq1, const std::vector<KmerPos>& sorted_kmers_seq2);
std::vector<KmerHit> findKmerMatches(const std::string& seq1, const std::string& seq2, int32_t k);
std::vector<KmerHit> findKmerMatches(const std::string& seq1, const std::string& seq2, int32_t k);

#endif
