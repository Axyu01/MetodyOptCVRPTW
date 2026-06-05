#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
  Solution(Solution *s);
  Solution(int size);
  Solution(Solution *s, int size);
  virtual ~Solution();
  int *Genome;
  int size = 0;
  double eval = 0;
  double penalty = 0;
  void print();
  void Insert(int g, int position);
  void Remove(int position);

protected:
private:
};

#endif // SOLUTION_H
