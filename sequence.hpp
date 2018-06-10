#ifndef MONITOR_SEQUENCE_DATA_HPP
#define MONITOR_SEQUENCE_DATA_HPP

#include <map>
#include <tuple>

// Efficient datastructure for storing meta-data for large sequences of indices,
// where adjacent indices are very likely to have the same meta-data. Built with
// storing meta-data for bytes in TCP in mind. Indices are 32-bit unsigned
// integers that wrap around. Assumes bit-by-bit comparison of the meta-data is
// a valid test for equality.
class Sequence {
public:
  typedef uint32_t Seq;
  // Starting sequence -> length and data
  typedef std::map<Seq, std::pair<Seq, uint8_t*>> DataMap;

  // Initialize. md_size is the size (in bytes) of the metadata to store.
  Sequence(size_t md_size);
  // Set indices [start, start+len) to metadata md, overwriting existing data if
  // any.
  void set_block(Seq start, Seq len, uint8_t* md);
  // Get the metadata at the position. Returns nullptr if none found.
  const uint8_t* get_md(Seq pos);

  // Return the entire datastructure, in internal representation. For testing
  // and debugging only.
  const DataMap get_all_data() const {return data;};

private:
  // Return iterator to a block containing (or ending at, but NOT containing)
  // pos, if it exists. Else return data.end()
  DataMap::iterator get_block(Seq pos);
  // Whether pos is within [start, start+len), taking into account wraparound
  bool is_within_range(Seq pos, Seq start, Seq len) const;

  size_t md_size;
  DataMap data;
};

#endif // o#ifndef MONITOR_SEQUENCE_DATA_HPP
