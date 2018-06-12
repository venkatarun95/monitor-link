#include "sequence.hpp"

#include <iostream>

using namespace std;

struct Md {
  int x;
  Md(int x): x(x) {}
  operator uint8_t*() {return (uint8_t*)this;}
};

void print(Sequence& seq) {
  for (auto& x : seq.get_all_data())
    cout << "(" << x.first << ", " << x.first + x.second.first << "): "
         << ((Md*)x.second.second)->x << " ";
  cout << endl;
}

int main() {
  Md md0(0), md1(1), md2(2), mdx(3);

  Sequence seq(sizeof(int));
  print(seq);
  seq.set_block(10, 10, md2);
  print(seq);

  // Attach exactly at the back
  seq.set_block(20, 10, md2);
  print(seq);
  seq.set_block(30, 20, md1);
  print(seq);

  // Independent block
  seq.set_block(100, 20, md1);
  print(seq);

  // Attach exactly at the front
  seq.set_block(5, 5, md2);
  print(seq);
  seq.set_block(2, 3, md1);
  print(seq);
  seq.set_block(90, 10, md1);
  print(seq);
  seq.set_block(80, 10, md2);
  print(seq);

  // Attach overlapping at the back
  seq.set_block(115, 15, md1);
  print(seq);
  seq.set_block(125, 15, md2);
  print(seq);

  // Attach overlapping at the front
  seq.set_block(70, 15, md2);
  print(seq);
  seq.set_block(60, 15, md1);
  print(seq);

  // Insert in the middle
  seq.set_block(100, 10, md1);
  print(seq);
  seq.set_block(100, 10, md2);
  print(seq);

  // Join blocks

  return 0;
}
