#include "rng.h"
#include "Solution.h"
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

// Deep copy
Solution::Solution(Solution *s) {
  size     = s->size;
  capacity = s->size;
  Genome   = new int[capacity];
  memcpy(Genome, s->Genome, size * sizeof(int));
  eval    = s->eval;
  penalty = s->penalty;
}

// Random permutation init (Fisher-Yates - O(n) instead of old O(n^2))
Solution::Solution(int size) {
  this->size = size;
  capacity   = size;
  Genome     = new int[capacity];
  for (int g = 0; g < size; g++) Genome[g] = g;
  for (int g = size - 1; g > 0; g--) {
    int j      = trand() % (g + 1);
    int tmp    = Genome[g];
    Genome[g]  = Genome[j];
    Genome[j]  = tmp;
  }
}

// Copy first min(s->size, size) genes, pad rest with sequential ints
Solution::Solution(Solution *s, int size) {
  this->size = size;
  capacity   = size;
  Genome     = new int[capacity];
  int copy   = s->size < size ? s->size : size;
  memcpy(Genome, s->Genome, copy * sizeof(int));
  for (int g = s->size; g < size; g++) Genome[g] = g;
  eval    = s->eval;
  penalty = s->penalty;
}

Solution::~Solution() { delete[] Genome; }

void Solution::print() {
  if (size <= 0 || Genome == nullptr) {
    cout << "\n[solution] empty";
    return;
  }
  cout << "\n[solution] genome: ";
  for (int g = 0; g < size - 1; g++) cout << Genome[g] << " -> ";
  cout << Genome[size - 1];
  cout << " | nodes: " << size << " | cost: " << eval
       << " | penalty: " << penalty;
}

// Insert with capacity doubling - no realloc when buffer has room
void Solution::Insert(int gene, int position) {
  if (position < 0)    position = 0;
  if (position > size) position = size;

  if (size >= capacity) {
    capacity      = capacity > 0 ? capacity * 2 : 4;
    int *newGenome = new int[capacity];
    memcpy(newGenome, Genome, size * sizeof(int));
    delete[] Genome;
    Genome = newGenome;
  }

  // shift right in-place
  for (int g = size; g > position; g--) Genome[g] = Genome[g - 1];
  Genome[position] = gene;
  size++;
  eval = 0; penalty = 0;
}

// Remove with in-place shift - never reallocates
void Solution::Remove(int position) {
  if (position < 0)      position = 0;
  if (position >= size)  position = size - 1;

  // shift left in-place
  for (int g = position; g < size - 1; g++) Genome[g] = Genome[g + 1];
  size--;
  eval = 0; penalty = 0;
}
