#ifndef TESTS_H
#define TESTS_H
#include "Problem.h"
#include <string>


class Tests
{
    public:
        Tests();
        virtual ~Tests();
        static void EvoConfigTest();
        static void EvoTest(Problem* problem,
        std::string test_id,
        int popSize,
        int Xp,
        int Mp,
        int turSize,
        int elitesNum,
        int MUT_ID,
        int CROSS_ID);
        static void EvoTest(Problem* problem,int iteration);
        static void EvoParamTest();
        static void EvoTest();
        static void GreedyTest(Problem* problem);
        static void GreedyTest();
        static void RandomTest(Problem* problem);
        static void RandomTest();
        static void SATest(Problem* problem,
        std::string test_id,
        float startTemp,
        float coolingFactor,
        int tempIterations,
        int MUT_ID);
        static void SAParamTest();
        static void SATest(Problem* problem);
        static void SATest();

    protected:

    private:
        const static int STANDARD_MULTIPLAYER = 10;//10;
        const static int OVER500_MULTIPLAYER = 100;//100;
};

#endif // TESTS_H
