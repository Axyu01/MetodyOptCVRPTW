#ifndef EVOTEST_H
#define EVOTEST_H

#include "EvoAlg.h"
#include "Problem.h"
#include <string>

class EVOTest {
public:
  static void UniversalTest(Problem *problem, EvoAlg *evo, std::string test_id);
  const static int STANDARD_MULTIPLAYER = 200;
  const static int OVER500_MULTIPLAYER = 100;

protected:
private:
};

#endif // EVOTEST_H
