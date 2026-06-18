#include <iostream>
#include "Problem.h"
#include "GreadySolver.h"
#include <string>
#include <cstdlib>
#include <time.h>
#include <algorithm>
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
        double avg = evo->GetAvarage();
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
// Returns a copy of the best solution found by trying every permutation.
// Only practical for tiny instances (up to ~8 customers).
Solution* brute_force(Problem* problem)
{
    int n = problem->SIZE;
    int* perm = new int[n];
    for (int i = 0; i < n; i++) perm[i] = i;

    Solution* best = nullptr;
    do {
        Solution* s = new Solution(n);
        for (int i = 0; i < n; i++) s->Genome[i] = perm[i];
        problem->EstimateSolution(s);
        if (best == nullptr || s->eval < best->eval) {
            delete best;
            best = s;
        } else {
            delete s;
        }
    } while (next_permutation(perm, perm + n));

    delete[] perm;
    return best;
}

void verify_tiny(const string& path, int size)
{
    cout << "\n==============================" << endl;
    cout << "Instance: " << path << endl;

    Problem* problem = new Problem(path, size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER = 0.01;

    // Brute-force optimal
    Solution* opt = brute_force(problem);
    cout << "[brute force] optimal:";
    opt->print();
    double optimal_cost = opt->eval;
    delete opt;

    // EA run
    test_evo(problem, -1, false);  // -1 = no debug checkpoints

    // Final best from a second run we can inspect
    int popSize = 33;
    EvoAlg* evo = new EvoAlg(problem, popSize);
    int budget = problem->SIZE * problem->SIZE * EVOTest::STANDARD_MULTIPLAYER;
    evo->Xp = 75; evo->Mp = 25; evo->REPAIRp = 70;
    evo->OPTp = 30; evo->turSize = 2; evo->REDISTp = 80;
    evo->REDIST_TRIES = 8; evo->elitesNum = 1;
    evo->MUT_ID = MutationOps::SWAP_ID;
    evo->CROSS_ID = CrossOps::OX_ID;
    evo->Init();
    evo->Eval();
    int loops = budget / popSize;
    for (int i = 0; i < loops; i++) {
        evo->Evolve();
        evo->Eval();
    }
    Solution* ea_best = evo->GetBest();
    cout << "[ea]          best found:";
    ea_best->print();

    double ea_cost = ea_best->eval;
    if (ea_cost <= optimal_cost + 1e-6)
        cout << "\n[PASS] EA matched optimal (" << optimal_cost << ")" << endl;
    else
        cout << "\n[FAIL] EA got " << ea_cost << ", optimal is " << optimal_cost << endl;

    delete ea_best;
    delete evo;
    delete problem;
}

EvoAlg* make_evo(Problem* problem, int popSize)
{
    EvoAlg* evo = new EvoAlg(problem, popSize);
    evo->Xp      = 75;
    evo->Mp      = 25;
    evo->REPAIRp = 70;
    evo->OPTp    = 30;
    evo->REDISTp = 80;
    evo->REDIST_TRIES = 8;
    evo->turSize  = 2;
    evo->elitesNum = 1;
    evo->MUT_ID   = MutationOps::SWAP_ID;
    evo->CROSS_ID = CrossOps::OX_ID;
    return evo;
}

void test_solomon(const string& path, int size, double bks, int loops = 500)
{
    cout << "\n==============================" << endl;
    Problem* problem = new Problem(path, size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = 0.01;

    EvoAlg* evo = make_evo(problem, 50);
    evo->Init();
    evo->Eval();

    for (int i = 0; i < loops; i++) {
        evo->Evolve();
        evo->Eval();
    }

    Solution* best = evo->GetBest();
    double gap = 100.0 * (best->eval - bks) / bks;
    cout << "Instance: " << problem->NAME
         << "  loops: " << loops
         << "  best: " << best->eval
         << "  BKS: " << bks
         << "  gap: " << gap << "%" << endl;
    delete best;
    delete evo;
    delete problem;
}

int main()
{
    srand(time(0));

    // Tiny benchmark sanity check
    verify_tiny("./problems/tiny/tiny3.txt",         3);
    verify_tiny("./problems/tiny/tiny3_shifted.txt", 3);
    verify_tiny("./problems/tiny/tiny4.txt",         4);
    verify_tiny("./problems/tiny/tiny5_square.txt",  5);
    verify_tiny("./problems/tiny/tiny5_tw.txt",      5);

    // Solomon 100-customer benchmarks
    // BKS values from the standard Solomon benchmark tables
    test_solomon("./problems/solomon-100/c101.txt",  100, 827.3);
    test_solomon("./problems/solomon-100/r101.txt",  100, 1650.8);
    test_solomon("./problems/solomon-100/rc101.txt", 100, 1696.94);

    return 0;
}
