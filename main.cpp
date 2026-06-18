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
#include "SAAlg.h"
#include "EVOTest.h"
#include "cstdio"
using namespace std;
void test_opt()
{
    Problem* problem = new Problem("problems/tai4_5_0.fsp",5);
    Solution* s_opt = new Solution(problem->PREFFERED_GENOME_SIZE);
    s_opt->Genome[0] = 0;
    s_opt->Genome[1] = 2;
    s_opt->Genome[2] = 3;
    s_opt->Genome[3] = 1;
    cout << "[test_opt] cost: " << problem->EstimateSolution(s_opt) << endl;
    s_opt->print();
}
void test_evo(Problem* problem,int debug_interval,bool isInfinite)
{
    cout<<endl<<endl<<"[evo] evolutionary test"<<endl;
    int popSize = 33;//50,100,500
    int Xp = 75;
    int Mp = 25;
    int REPAIRp = 70;
    bool REPAIR_TRY_BEFORE = false;
    bool REPAIR_TRY_AFTER = false;
    bool REPAIR_USE_DEMAND = false;
    int OPTp = 30;
    int OPT_TRACK_MAX_LOCATION_COUNT = 1;
    int REDISTp = 80;
    int REDIST_TRIES = 8;
    int turSize = 2;
    int elitesNum = 1;
    int MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
    int CROSS_ID = CrossOps::OX_ID;

    EvoAlg* evo = new EvoAlg(problem,popSize);
    int budget = problem->SIZE*problem->SIZE*EVOTest::STANDARD_MULTIPLAYER;
    if(problem->SIZE>=500)
        budget = problem->SIZE*EVOTest::OVER500_MULTIPLAYER;
    evo->Xp = Xp;
    evo->Mp = Mp;
    evo->REPAIRp = REPAIRp;
    evo->REPAIR_TRY_BEFORE = REPAIR_TRY_BEFORE;
    evo->REPAIR_TRY_AFTER = REPAIR_TRY_AFTER;
    evo->REPAIR_USE_DAMAND = REPAIR_USE_DEMAND;
    evo->OPTp = OPTp;
    evo->turSize = turSize;
    evo->REDISTp = REDISTp;
    evo->REDIST_TRIES = REDIST_TRIES;
    evo->elitesNum = elitesNum;
    evo->MUT_ID = MUT_ID;
    evo->CROSS_ID = CROSS_ID;


    evo->Init();
    evo->Eval();
    cout << "[evo] initial best:";
    evo->GetBest()->print();
    cout << endl << "[evo] initial worst:";
    evo->GetWorst()->print();
    cout << endl << "[evo] initial average cost: " << evo->GetAvarage() << endl;
    string file_name = "out/evo_";
    file_name += problem->NAME;

    //Logger bestLogger(file_name+"_best.csv");
    //Logger worstLogger(file_name+"_worst.csv");
    //Logger avgLogger(file_name+"_avg.csv");

    int loops = budget/popSize;
    for(int i=0;i<loops || isInfinite;i++)
    {
        //Evo Loop
        evo->Evolve();
        evo->Eval();
        //Debugging
        if(debug_interval >0 && i%debug_interval == 0)
        {
            cout<<endl<<endl<<"[evo] checkpoint " << i << " of " << loops << endl;
            cout << "[evo] average cost: " << evo->GetAvarage() << endl;
            cout << "[evo] best solution:";
            Solution* best = evo->GetBest();
            best->print();
            delete best;
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
    cout<<endl<<"[evo] final best:";
    evo->GetBest()->print();
    cout<<endl<<"[evo] final worst:";
    evo->GetWorst()->print();
    cout<<endl<<"[evo] final average cost: "<<evo->GetAvarage();
}
void test_random(Problem* problem)
{
    int tests = 10000;
    long long int sum = 0;
    Solution* best = nullptr;
    for(int i=0;i<tests;i++)
    {
        Solution* s = new Solution(problem->PREFFERED_GENOME_SIZE);
        sum += problem->EstimateSolution(s);
        if(best == nullptr || s->eval < best->eval)
        {
            delete best;
            best = s;
        }
        else
            delete s;
    }
    cout <<"[random] average cost: "<< sum/tests << endl;
    best->print();
    delete best;
}
void test_sa(Problem* problem)
{
    cout <<endl<<endl<<"[sa] simulated annealing test"<<endl;
    float startTemp = problem->PREFFERED_GENOME_SIZE*problem->PREFFERED_GENOME_SIZE*problem->PREFFERED_GENOME_SIZE;
    SAAlg sa(problem,100);

    int budget = problem->SIZE*problem->SIZE*EVOTest::STANDARD_MULTIPLAYER;
    if(problem->SIZE>=500)
        budget = problem->SIZE*EVOTest::OVER500_MULTIPLAYER;

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
    cout << "[sa] best solution:";
    sa.bestSolution->print();
}
int main()
{
    srand(time(0));

    Problem* problem = new Problem("./problems/tiny/tiny3.txt",3);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER = 0.01;
    Solution* s = new Solution(problem->SIZE);
    for(int i =0;i<s->size;i++)
    {
        s->Genome[i] = i;
    }
    cout << "[main] baseline route evaluation" << endl;
    problem->EstimateSolution(s);
    s->print();
    //Solution* opt = MutationOps::OptimalTrack(problem,s);
    //opt->print();

    //test_sa(problem);
    test_evo(problem,100,false);

    return 0;
}
