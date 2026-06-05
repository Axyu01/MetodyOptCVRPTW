#ifndef SAALG_H
#define SAALG_H
#include "MutationOps.h"
#include "Problem.h"
#include "Solution.h"

class SAAlg {
  const int MAX_PROB = 100;

public:
  SAAlg(Problem *problem, float startTemp);
  virtual ~SAAlg();
  bool Iterate();
  Solution *bestSolution = nullptr;
  Solution *currentSolution = nullptr;
  int MUT_ID = MutationOps::SWAP_ID;
  int tempIterations;
  float maxIterations;
  float coolingFactor;
  int currentIteration = 0;
  float currentTemp;

protected:
  Problem *problem;

private:
};

#endif // SAALG_H
