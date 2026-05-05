#include <iostream>
#include "Problem.h"
#include "GreadySolver.h"
#include <string>
#include <cstdlib>
#include <time.h>
#include "CrossOps.h"
#include "MutationOps.h"
#include "EvoAlg.h"
#include "Logger.h"
#include "Tests.h"
#include "SAAlg.h"
using namespace std;
void test_opt()
{
    Problem* problem = new Problem("problems/tai4_5_0.fsp");
    Solution* s_opt = new Solution(problem->J);
    s_opt->Genome[0] = 0;
    s_opt->Genome[1] = 2;
    s_opt->Genome[2] = 3;
    s_opt->Genome[3] = 1;
    cout << problem->EstimateSolution(s_opt) <<endl;
    s_opt->print();
}
void test_gready_and_operators(Problem* problem)
{
    GreadySolver* greadySolver = new GreadySolver(problem);
    Solution* greadySolution = greadySolver->GetSolution(0);
    greadySolution->eval = problem->EstimateSolution(greadySolution);
    cout << problem->EstimateSolution(greadySolution) <<endl;

    Solution* s_random = new Solution(problem->J);


    greadySolution->print();
    s_random->print();
    MutationOps::Inverse(greadySolution)->print();
    CrossOps::PMX(greadySolution,s_random,problem->J)->print();
}
void test_evo(Problem* problem)
{
    int popSize = 33;//50,100,500
    int Xp = 75;
    int Mp = 25;
    int turSize = 2;
    int elitesNum = 1;
    int MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
    int CROSS_ID = CrossOps::OX_ID;

    EvoAlg* evo = new EvoAlg(problem,popSize);
    int budget = problem->J*problem->J*10;
    if(problem->J>=500)
        budget = problem->J*100;
    evo->Xp = Xp;
    evo->Mp = Mp;
    evo->turSize = turSize;
    evo->elitesNum = elitesNum;
    evo->MUT_ID = MUT_ID;
    evo->CROSS_ID = CROSS_ID;
    evo->Init();
    evo->Eval();
    evo->GetBest()->print();
    evo->GetWorst()->print();
    cout<<endl<<evo->GetAvarage();
    string file_name = "out/evo_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);

    //Logger bestLogger(file_name+"_best.csv");
    //Logger worstLogger(file_name+"_worst.csv");
    //Logger avgLogger(file_name+"_avg.csv");

    int loops = budget/popSize;
    for(int i=0;i<loops;i++)
    {
        //Evo Loop
        evo->Evolve();
        evo->Eval();
        //Debugging
        if(i%100 == 0 && false)
        {
            cout<<endl<<evo->GetAvarage();
            evo->GetBest()->print();
        }
        //Logging
        Solution* best = evo->GetBest();
        Solution* worst = evo->GetWorst();
        int avg = evo->GetAvarage();
        /*bestLogger.Log(best);
        worstLogger.Log(worst);
        avgLogger.Log(avg);*/
        delete best;
        delete worst;
    }
    evo->GetBest()->print();
    evo->GetWorst()->print();
    cout<<endl<<evo->GetAvarage();
}
void test_random(Problem* problem)
{
    int tests = 10000;
    long long int sum = 0;
    for(int i=0;i<tests;i++)
    {
        Solution* s = new Solution(problem->J);
        sum += problem->EstimateSolution(s);
        delete s;
    }
    cout <<"Random avg:"<< sum/tests << endl;
}
void test_sa(Problem* problem)
{
    float startTemp = problem->J*problem->J*problem->M*problem->M;
    SAAlg sa(problem,100);

    int budget = problem->J*problem->J*10;
    if(problem->J>=500)
        budget = problem->J*100;

    sa.coolingFactor = 0.995;
    sa.maxIterations = budget;//problem->J * 1000;
    sa.tempIterations = 25;//problem->J;
    sa.MUT_ID = MutationOps::SWAP_ID;
    int best = sa.bestSolution->eval;
    while(sa.Iterate())
    {
        if(best != sa.bestSolution->eval)
        {
            best = sa.bestSolution->eval;
            //cout <<endl<<sa.currentIteration<<"/"<<sa.maxIterations<<" | temp: "<<sa.currentTemp<<" |new best:"<<best<<endl;
        }
    }
    sa.bestSolution->print();
}
int main()
{
    srand(time(0));

    Problem* problem = new Problem("problems/tai100_20_0.fsp");

    //test_gready_and_operators(problem);
    //test_random(problem);
    //test_evo(problem);
    //test_sa(problem);
    Tests::RandomTest();
    //Tests::GreedyTest();
    //Tests::EvoTest();
    //Tests::EvoParamTest();
    //Tests::SATest();
    //Tests::SAParamTest();
    //Tests::SATest(problem);

    return 0;
}
