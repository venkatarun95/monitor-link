#include "sequence.hpp"

#include <iostream>

using namespace std;

Sequence::Sequence(size_t md_size) :
  md_size(md_size),
  data()
{}

bool Sequence::is_within_range(Seq pos, Seq start, Seq len) const {
  if (len == 0)
    return false;
  return ((start < start + len && start < pos && pos < start + len) ||
          (start + len < start && (start < pos || pos < start + len)));
}

void Sequence::set_block(Seq start, Seq len, uint8_t* md) {
  if (len == 0)
    return;

  auto start_block = get_block(start);

  // Nothing exists there. Construct a new block
  if (start_block == data.end()) {
    cerr << "Construct new block" << endl;
    uint8_t* new_md = new uint8_t[md_size];
    memcpy(new_md, md, md_size);
    data[start] = make_pair(len, new_md);
    start_block = data.find(start);
    //return;
  }
  else {
    auto next_block = ++ start_block;
    -- start_block;
    // Usual case where new block follows an existing block. Handle specially for
    // speed.
    if (start_block->first + start_block->second.first == start &&
        next_block->first != start + len) {
      cerr << "Common case" << endl;
      // If meta-data is equal, just extend the block
      if (memcmp(start_block->second.second, md, md_size) == 0) {
        start_block->second.first += len;
      }
      // Create new block
      else {
        uint8_t* new_md = new uint8_t[md_size];
        memcpy(new_md, md, md_size);
        data[start] = make_pair(len, new_md);      
      }
      return;
    }

    // We are inside a block. Rewrite if necessary
    // If metadata of block we are in is the same, extend
    if (memcmp(start_block->second.second, md, md_size) == 0) {
      cerr << "Extend" << endl;
      // Extend only if current addition is bigger than existing block
      if (!is_within_range(start + len, start_block->first,
                          start_block->second.first))
        start_block->second.first = start + len - start_block->first;
    }
    // If metadata of block we are in is different, chop that block and start a
    // new one.
    else {
      // If chopping empties that block, convert it into our new block
      if (start_block->first == start) {
        start_block->second.first = len;
        memcpy(start_block->second.second, md, md_size);
      }
      // Chop and create a new block
      else {
        cerr << "Dice" << endl;
        start_block->second.first = start - start_block->first;
        uint8_t* new_md = new uint8_t[md_size];
        memcpy(new_md, md, md_size);
        data[start] = make_pair(len, new_md);
        start_block = data.find(start);
      }
    }
  }

  cerr << "Block: " << start_block->first << " "
       << start_block->first + start_block->second.first << endl;
  auto next_block = ++ start_block;
  -- start_block;

  // If the next block is contiguous and its metadata is the same, chop this
  // block and extend the next one backwards. Handling this case separately
  // isn't necessary for correctness, but it is more efficient to handle
  // separately as we avoid unnecessary memory allocation/deallocation.
  if (start+len >= next_block->first &&
      start+len < next_block->first + next_block->second.first &&
      memcmp(next_block->second.second, md, md_size) == 0) {
    cerr << "Merge with next block" << endl;
    // Extend backwards
    data[start] = next_block->second; // TODO(venkat): check
    data[start].first = next_block->first + next_block->second.first - start;
    data.erase(next_block);
    // Chop the current block
    //start_block->second.first = start - start_block->first;
    return;
  }

  // Now start_block points to our block. But if it extends into later blocks,
  // we want to delete those blocks, and chop the last one.
  auto block_it = ++ start_block;
  -- start_block;
  // Start of block_it is within start_block
  while (is_within_range(block_it->first, start, len)) {
    cerr << "Exploring " << block_it->first << " " << block_it->first + block_it->second.first << endl;
    // Wrap around. Warning: Be careful or could cause infinite loop
    if (block_it == data.end())
      block_it = data.begin(); 

    // If it ends before start_block, just delete it
    if (is_within_range(block_it->first+block_it->second.first-1, start, len))
      block_it = data.erase(block_it);
    // Else chop it from the start
    else {
      cerr << "Chopping" << endl;
      data[start+len] = block_it->second;
      data[start+len].first = block_it->first + block_it->second.first
        - (start + len);
      block_it = data.erase(block_it);
      continue;
    }
    ++ block_it;
  }
}

const uint8_t* Sequence::get_md(Seq pos) {
  auto res = get_block(pos);
  if (res == data.end())
    return nullptr;
  // get_block returns even if the following condition holds, which we don't
  // want here
  else if (pos == res->first + res->second.first)
    return nullptr;
  else return res->second.second;
}

Sequence::DataMap::iterator Sequence::get_block(Seq pos) {
  if (data.empty())
    return data.end();

  auto it = data.lower_bound(pos);
  // Check the last block before giving up
  if (it == data.end()) {
    auto it_end = data.rbegin();
    if ((it_end->first < pos && pos <= it_end->first + it_end->second.first) ||
        (it_end->first + it_end->second.first < it_end->first &&
         (it_end->first < pos || pos < it_end->first + it_end->second.first)))
      return data.find(it_end->first);
    else
      return data.end();
  }

  if (it->first == pos)
    return it;

  // Go back
  if (it == data.begin())
    return data.end();
  -- it;

  // Simple case. 'pos' is contained in range
  if (it->first <= pos && it->first + it->second.first >= pos)
    return it;
  // We got the beginning, maybe we have a wraparound
  // if (it == data.begin()) {
  //   auto it_end = data.rbegin();
  //   if (it_end->first + it_end->second.first < pos)
  //     return data.find(it_end->first);
  // }
  return data.end();
}

