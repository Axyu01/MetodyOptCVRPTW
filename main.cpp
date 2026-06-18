#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include "Problem.h"
#include "GreadySolver.h"
#include "CrossOps.h"
#include "MutationOps.h"
#include "EvoAlg.h"
#include "Logger.h"
#include "SAAlg.h"
#include "EVOTest.h"
using namespace std;

// -------------------------------------------------------
// Brute-force (tiny instances only, up to ~8 customers)
// -------------------------------------------------------
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
        if (best == nullptr || s->eval < best->eval) { delete best; best = s; }
        else { delete s; }
    } while (next_permutation(perm, perm + n));
    delete[] perm;
    return best;
}

// -------------------------------------------------------
// Tiny benchmark verification
// -------------------------------------------------------
void verify_tiny(const string& path, int size)
{
    cout << "\n==============================" << endl;
    cout << "Instance: " << path << endl;

    Problem* problem = new Problem(path, size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = 0.01;

    Solution* opt = brute_force(problem);
    cout << "[brute force] optimal:"; opt->print();
    double optimal_cost = opt->eval;
    delete opt;

    int popSize = 33;
    EvoAlg* evo = new EvoAlg(problem, popSize);
    int budget = problem->SIZE * problem->SIZE * EVOTest::STANDARD_MULTIPLAYER;
    evo->Xp = 75; evo->Mp = 25; evo->REPAIRp = 70;
    evo->OPTp = 30; evo->turSize = 2; evo->REDISTp = 80;
    evo->REDIST_TRIES = 8; evo->elitesNum = 1;
    evo->MUT_ID   = MutationOps::SWAP_ID;
    evo->CROSS_ID = CrossOps::OX_ID;
    evo->Init(); evo->Eval();
    int loops = budget / popSize;
    for (int i = 0; i < loops; i++) { evo->Evolve(); evo->Eval(); }
    Solution* ea_best = evo->GetBest();
    cout << "[ea]          best:"; ea_best->print();
    double ea_cost = ea_best->eval;
    if (ea_cost <= optimal_cost + 1e-6)
        cout << "[PASS] EA matched optimal (" << optimal_cost << ")" << endl;
    else
        cout << "[FAIL] EA got " << ea_cost << ", optimal is " << optimal_cost << endl;

    delete ea_best; delete evo; delete problem;
}

// -------------------------------------------------------
// Tuning infrastructure
// -------------------------------------------------------

// Runs one trial, logs gen;best;avg;worst per generation, returns final best cost.
double run_experiment(Problem* problem, EvoAlg* evo, const string& csv_path, int budget)
{
    Logger log(csv_path);
    log.Log(0, evo->GetBest()->eval, evo->GetAvarage(), evo->GetWorst()->eval);

    int loops = budget / evo->popSize;
    for (int i = 1; i <= loops; i++) {
        evo->Evolve();
        evo->Eval();
        Solution* best  = evo->GetBest();
        Solution* worst = evo->GetWorst();
        double avg      = evo->GetAvarage();
        log.Log(i, best->eval, avg, worst->eval);
        delete best; delete worst;
    }
    Solution* final_best = evo->GetBest();
    double cost = final_best->eval;
    delete final_best;
    return cost;
}

struct EvoConfig {
    int popSize;
    int Xp;
    int Mp;
    int turSize;
    int elitesNum;
    int MUT_ID;
    int CROSS_ID;
    // custom operators
    int REPAIRp;
    int OPTp;
    int OPT_TRACK_MAX_LOCATION_COUNT;
    int REDISTp;
    int REDIST_TRIES;
};

EvoConfig base_config_with_ops() {
    EvoConfig c;
    c.popSize     = 50;
    c.Xp          = 75;
    c.Mp          = 25;
    c.turSize     = 2;
    c.elitesNum   = 1;
    c.MUT_ID      = MutationOps::SWAP_ID;
    c.CROSS_ID    = CrossOps::OX_ID;
    c.REPAIRp     = 70;
    c.OPTp        = 30;
    c.OPT_TRACK_MAX_LOCATION_COUNT = 1;
    c.REDISTp     = 80;
    c.REDIST_TRIES = 8;
    return c;
}

EvoConfig base_config_no_ops() {
    EvoConfig c = base_config_with_ops();
    c.REPAIRp  = 0;
    c.OPTp     = 0;
    c.REDISTp  = 0;
    return c;
}

EvoAlg* make_evo(Problem* problem, const EvoConfig& cfg)
{
    EvoAlg* evo = new EvoAlg(problem, cfg.popSize);
    evo->Xp          = cfg.Xp;
    evo->Mp          = cfg.Mp;
    evo->turSize     = cfg.turSize;
    evo->elitesNum   = cfg.elitesNum;
    evo->MUT_ID      = cfg.MUT_ID;
    evo->CROSS_ID    = cfg.CROSS_ID;
    evo->REPAIRp     = cfg.REPAIRp;
    evo->OPTp        = cfg.OPTp;
    evo->OPT_TRACK_MAX_LOCATION_COUNT = cfg.OPT_TRACK_MAX_LOCATION_COUNT;
    evo->REDISTp     = cfg.REDISTp;
    evo->REDIST_TRIES = cfg.REDIST_TRIES;
    return evo;
}

// Runs N independent trials for a config on one instance.
// csv files: out/{instance}_{tag}_{run}.csv
// Returns average final best cost.
double run_config(const string& instance_path, int size, const string& tag,
                  const EvoConfig& cfg, int n_runs, int budget)
{
    Problem* problem = new Problem(instance_path, size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = 0.01;

    double sum = 0.0;
    double best_of_all = -1.0;
    for (int r = 0; r < n_runs; r++) {
        EvoAlg* evo = make_evo(problem, cfg);
        evo->Init(); evo->Eval();
        string csv = "out/" + problem->NAME + "_" + tag + "_" + to_string(r) + ".csv";
        double cost = run_experiment(problem, evo, csv, budget);
        sum += cost;
        if (best_of_all < 0 || cost < best_of_all) best_of_all = cost;
        delete evo;
    }
    double avg = sum / n_runs;
    cout << fixed << setprecision(2)
         << "  " << tag << "  avg=" << avg << "  best=" << best_of_all << endl;
    delete problem;
    return avg;
}

// -------------------------------------------------------
// Tuning experiment
// -------------------------------------------------------
void run_tuning()
{
    const int N_RUNS  = 5;
    // tuning budget: SIZE*SIZE = 10000 evals (fast enough to iterate over many configs)
    const int T_BUDGET = 100 * 100;

    struct Instance { string path; int size; string name; };
    Instance instances[] = {
        {"./problems/solomon-100/c101.txt",  100, "c101"},
        {"./problems/solomon-100/r101.txt",  100, "r101"},
        {"./problems/solomon-100/rc101.txt", 100, "rc101"},
    };

    // ----------------------------------------------------------
    // Phase 1: Operators ON vs OFF (base params)
    // ----------------------------------------------------------
    cout << "\n========== Phase 1: Custom Operators ON vs OFF ==========" << endl;
    for (auto& inst : instances) {
        cout << "\nInstance: " << inst.name << endl;
        run_config(inst.path, inst.size, "ops_off", base_config_no_ops(),  N_RUNS, T_BUDGET);
        run_config(inst.path, inst.size, "ops_on",  base_config_with_ops(), N_RUNS, T_BUDGET);
    }

    // ----------------------------------------------------------
    // Phase 2: Population size (operators ON)
    // ----------------------------------------------------------
    cout << "\n========== Phase 2: Population size (ops ON) ==========" << endl;
    int pop_values[] = {10, 20, 50, 100};
    for (auto& inst : instances) {
        cout << "\nInstance: " << inst.name << endl;
        for (int pop : pop_values) {
            EvoConfig cfg = base_config_with_ops();
            cfg.popSize = pop;
            run_config(inst.path, inst.size,
                       "pop" + to_string(pop),
                       cfg, N_RUNS, T_BUDGET);
        }
    }

    // ----------------------------------------------------------
    // Phase 3: Crossover probability Xp (ops ON, best pop)
    // Use pop=50 as default for this phase (review Phase 2 results to update)
    // ----------------------------------------------------------
    cout << "\n========== Phase 3: Crossover probability Xp (ops ON) ==========" << endl;
    int xp_values[] = {50, 75, 90};
    for (auto& inst : instances) {
        cout << "\nInstance: " << inst.name << endl;
        for (int xp : xp_values) {
            EvoConfig cfg = base_config_with_ops();
            cfg.Xp = xp;
            run_config(inst.path, inst.size,
                       "xp" + to_string(xp),
                       cfg, N_RUNS, T_BUDGET);
        }
    }

    // ----------------------------------------------------------
    // Phase 4: Mutation probability Mp (ops ON, best pop + Xp)
    // ----------------------------------------------------------
    cout << "\n========== Phase 4: Mutation probability Mp (ops ON) ==========" << endl;
    int mp_values[] = {10, 25, 40};
    for (auto& inst : instances) {
        cout << "\nInstance: " << inst.name << endl;
        for (int mp : mp_values) {
            EvoConfig cfg = base_config_with_ops();
            cfg.Mp = mp;
            run_config(inst.path, inst.size,
                       "mp" + to_string(mp),
                       cfg, N_RUNS, T_BUDGET);
        }
    }

    // ----------------------------------------------------------
    // Phase 5: Operators ON vs OFF with FULL budget (best params)
    // Use 20x tuning budget: SIZE*SIZE*20 = 200,000 evals
    // ----------------------------------------------------------
    cout << "\n========== Phase 5: Final ON vs OFF (full budget) ==========" << endl;
    const int F_BUDGET = 100 * 100 * 20;
    for (auto& inst : instances) {
        cout << "\nInstance: " << inst.name << endl;
        run_config(inst.path, inst.size, "final_ops_off", base_config_no_ops(),  N_RUNS, F_BUDGET);
        run_config(inst.path, inst.size, "final_ops_on",  base_config_with_ops(), N_RUNS, F_BUDGET);
    }
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------
int main()
{
    srand(time(0));

    // Tiny sanity checks (uncomment to run)
    // verify_tiny("./problems/tiny/tiny3.txt",         3);
    // verify_tiny("./problems/tiny/tiny3_shifted.txt", 3);
    // verify_tiny("./problems/tiny/tiny4.txt",         4);
    // verify_tiny("./problems/tiny/tiny5_square.txt",  5);
    // verify_tiny("./problems/tiny/tiny5_tw.txt",      5);

    run_tuning();

    return 0;
}
