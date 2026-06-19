#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include "Problem.h"
#include "CrossOps.h"
#include "MutationOps.h"
#include "EvoAlg.h"
#include "Logger.h"
#include "EVOTest.h"
using namespace std;

// ─── Constants ───────────────────────────────────────────────────────────────

const int    N_RUNS     = 5;   // runs per config during tuning
const int    CMP_RUNS   = 10;  // runs per config during final comparison
const int    T_BUDGET = 300'000;   // evals per run during tuning

struct Instance { string path; int size; string name; };
const Instance INSTANCES[] = {
    {"./problems/solomon-100/c101.txt",  100, "c101"},
    {"./problems/solomon-100/r101.txt",  100, "r101"},
    {"./problems/solomon-100/rc101.txt", 100, "rc101"},
};
const int N_INSTANCES = 3;

// ─── EvoConfig ───────────────────────────────────────────────────────────────

struct EvoConfig {
    int popSize   = 50;
    int Xp        = 75;
    int Mp        = 25;
    int turSize   = 2;
    int elitesNum = 1;
    int MUT_ID    = MutationOps::SWAP_ID;
    int CROSS_ID  = CrossOps::OX_ID;
    // custom operators (0 = disabled)
    int REPAIRp   = 0;
    int OPTp      = 0;
    int REDISTp   = 0;
    int REDIST_TRIES               = 8;
    int OPT_TRACK_MAX_LOCATION_COUNT = 1;
};

EvoConfig ops_off_base() { return EvoConfig{}; }

EvoConfig ops_on_base() {
    EvoConfig c;
    c.REPAIRp = 70;
    c.OPTp    = 30;
    c.REDISTp = 80;
    return c;
}

// ─── EA factory ──────────────────────────────────────────────────────────────

EvoAlg* make_evo(Problem* problem, const EvoConfig& cfg) {
    EvoAlg* evo = new EvoAlg(problem, cfg.popSize);
    evo->Xp          = cfg.Xp;
    evo->Mp          = cfg.Mp;
    evo->turSize     = cfg.turSize;
    evo->elitesNum   = cfg.elitesNum;
    evo->MUT_ID      = cfg.MUT_ID;
    evo->CROSS_ID    = cfg.CROSS_ID;
    evo->REPAIRp     = cfg.REPAIRp;
    evo->OPTp        = cfg.OPTp;
    evo->REDISTp     = cfg.REDISTp;
    evo->REDIST_TRIES               = cfg.REDIST_TRIES;
    evo->OPT_TRACK_MAX_LOCATION_COUNT = cfg.OPT_TRACK_MAX_LOCATION_COUNT;
    return evo;
}

// ─── Core run ────────────────────────────────────────────────────────────────

// One trial: logs gen;best;avg;worst, returns final best cost.
double run_one(Problem* problem, const EvoConfig& cfg,
               const string& csv_path, int budget) {
    Logger log(csv_path);
    EvoAlg* evo = make_evo(problem, cfg);
    evo->Init(); evo->Eval();

    Solution* b0 = evo->GetBest(); Solution* w0 = evo->GetWorst();
    log.Log(0, b0->eval, evo->GetAvarage(), w0->eval);
    delete b0; delete w0;

    int loops = budget / cfg.popSize;
    for (int i = 1; i <= loops; i++) {
        evo->Evolve(); evo->Eval();
        Solution* b = evo->GetBest(); Solution* w = evo->GetWorst();
        log.Log(i, b->eval, evo->GetAvarage(), w->eval);
        delete b; delete w;
    }
    Solution* fin = evo->GetBest();
    double cost = fin->eval;
    delete fin; delete evo;
    return cost;
}

// Stagnation-based run: stops after stag_gens generations with no improvement,
// or after max_evals total evaluations (whichever comes first).
double run_one_stagnation(Problem* problem, const EvoConfig& cfg,
                          const string& csv_path,
                          int stag_gens, int max_evals) {
    Logger log(csv_path);
    EvoAlg* evo = make_evo(problem, cfg);
    evo->Init(); evo->Eval();

    Solution* b0 = evo->GetBest(); Solution* w0 = evo->GetWorst();
    double best_eval = b0->eval;
    log.Log(0, best_eval, evo->GetAvarage(), w0->eval);
    delete b0; delete w0;

    int no_improve = 0;
    int max_gens = max_evals / cfg.popSize;

    for (int i = 1; i <= max_gens && no_improve < stag_gens; i++) {
        evo->Evolve(); evo->Eval();
        Solution* b = evo->GetBest(); Solution* w = evo->GetWorst();
        if (b->eval < best_eval - 1e-9) {
            best_eval = b->eval;
            no_improve = 0;
        } else {
            no_improve++;
        }
        log.Log(i, b->eval, evo->GetAvarage(), w->eval);
        delete b; delete w;
    }
    Solution* fin = evo->GetBest();
    double cost = fin->eval;
    delete fin; delete evo;
    return cost;
}

// N_RUNS sequential trials on one instance. Returns mean final-best.
double run_instance(const Instance& inst, const EvoConfig& cfg,
                    const string& tag, int budget, const string& out_dir) {
    Problem* problem = new Problem(inst.path, inst.size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = 0.01;

    double sum = 0;
    for (int r = 0; r < N_RUNS; r++) {
        string csv = out_dir + "/" + inst.name + "_" + tag + "_" + to_string(r) + ".csv";
        sum += run_one(problem, cfg, csv, budget);
    }
    delete problem;
    return sum / N_RUNS;
}

// ─── Phase runner ────────────────────────────────────────────────────────────

struct Variant { string tag; EvoConfig cfg; };

struct PhaseResult {
    string winner_tag;
    EvoConfig winner_cfg;
    // per-instance avgs for the winner, indexed by INSTANCES order
    double inst_avgs[3];
    double overall_avg;
};

// Runs all variants on all 3 instances (N_RUNS each). Picks winner by lowest overall avg.
PhaseResult run_phase(const string& phase_name,
                      const vector<Variant>& variants,
                      int budget, const string& out_dir,
                      ofstream& summary) {
    cout << "\n--- " << phase_name << " ---" << endl;
    summary << "\n--- " << phase_name << " ---\n";

    PhaseResult best;
    best.overall_avg = 1e18;

    for (auto& v : variants) {
        double inst_avgs[3];
        double total = 0;
        for (int i = 0; i < N_INSTANCES; i++) {
            inst_avgs[i] = run_instance(INSTANCES[i], v.cfg, v.tag, budget, out_dir);
            total += inst_avgs[i];
        }
        double overall = total / N_INSTANCES;

        string line = "  " + v.tag
            + "  c101=" + to_string((int)inst_avgs[0])
            + "  r101=" + to_string((int)inst_avgs[1])
            + "  rc101=" + to_string((int)inst_avgs[2])
            + "  OVERALL=" + to_string((int)overall);
        if (overall < best.overall_avg) {
            line += "  <-- best";
            best.winner_tag = v.tag;
            best.winner_cfg = v.cfg;
            for (int i = 0; i < N_INSTANCES; i++) best.inst_avgs[i] = inst_avgs[i];
            best.overall_avg = overall;
        }
        cout << line << endl;
        summary << line << "\n";
    }
    cout << "  WINNER: " << best.winner_tag
         << "  (avg=" << fixed << setprecision(1) << best.overall_avg << ")" << endl;
    summary << "  WINNER: " << best.winner_tag
            << "  (avg=" << fixed << setprecision(1) << best.overall_avg << ")\n";
    return best;
}

// ─── Tuning pipelines ────────────────────────────────────────────────────────

EvoConfig tune_pipeline(EvoConfig base, bool with_ops,
                        int budget, const string& out_dir) {
    string label = with_ops ? "OPS_ON" : "OPS_OFF";
    string summary_path = out_dir + "/summary_" + label + ".txt";
    ofstream summary(summary_path);
    cout << "\n\n========================================" << endl;
    cout << "  PIPELINE: " << label << endl;
    cout << "========================================" << endl;
    summary << "PIPELINE: " << label << "\nBudget per run: " << budget
            << "  Runs: " << N_RUNS << "x3 instances\n";

    EvoConfig cfg = base;

    // Phase 1: population size
    {
        vector<Variant> vs;
        for (int pop : {10, 20, 50, 100}) {
            EvoConfig c = cfg; c.popSize = pop;
            vs.push_back({"pop" + to_string(pop), c});
        }
        cfg = run_phase("Phase 1: popSize", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 2: crossover type
    {
        vector<Variant> vs;
        for (auto [name, id] : vector<pair<string,int>>{{"OX",CrossOps::OX_ID},{"PMX",CrossOps::PMX_ID},{"CX",CrossOps::CX_ID}}) {
            EvoConfig c = cfg; c.CROSS_ID = id;
            vs.push_back({"cross_" + name, c});
        }
        cfg = run_phase("Phase 2: crossover", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 3: crossover probability Xp
    {
        vector<Variant> vs;
        for (int xp : {50, 75, 90}) {
            EvoConfig c = cfg; c.Xp = xp;
            vs.push_back({"xp" + to_string(xp), c});
        }
        cfg = run_phase("Phase 3: Xp", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 4: mutation probability Mp
    {
        vector<Variant> vs;
        for (int mp : {10, 25, 40}) {
            EvoConfig c = cfg; c.Mp = mp;
            vs.push_back({"mp" + to_string(mp), c});
        }
        cfg = run_phase("Phase 4: Mp", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 5: tournament size
    {
        vector<Variant> vs;
        for (int ts : {2, 3, 5}) {
            EvoConfig c = cfg; c.turSize = ts;
            vs.push_back({"tur" + to_string(ts), c});
        }
        cfg = run_phase("Phase 5: turSize", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 6 (ops_on only): REPAIRp
    if (with_ops) {
        vector<Variant> vs;
        for (int rp : {30, 70, 100}) {
            EvoConfig c = cfg; c.REPAIRp = rp;
            vs.push_back({"repair" + to_string(rp), c});
        }
        cfg = run_phase("Phase 6: REPAIRp", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 7 (ops_on only): OPTp
    if (with_ops) {
        vector<Variant> vs;
        for (int op : {10, 30, 60}) {
            EvoConfig c = cfg; c.OPTp = op;
            vs.push_back({"opt" + to_string(op), c});
        }
        cfg = run_phase("Phase 7: OPTp", vs, budget, out_dir, summary).winner_cfg;
    }

    // Phase 8 (ops_on only): REDISTp
    if (with_ops) {
        vector<Variant> vs;
        for (int rd : {30, 80, 100}) {
            EvoConfig c = cfg; c.REDISTp = rd;
            vs.push_back({"redist" + to_string(rd), c});
        }
        cfg = run_phase("Phase 8: REDISTp", vs, budget, out_dir, summary).winner_cfg;
    }

    // Print final best config
    auto cross_name = [](int id) -> string {
        if (id == CrossOps::OX_ID)  return "OX";
        if (id == CrossOps::PMX_ID) return "PMX";
        return "CX";
    };
    string final_line =
        "\nFINAL BEST CONFIG (" + label + "):\n"
        "  popSize="   + to_string(cfg.popSize)  + "\n"
        "  CROSS="     + cross_name(cfg.CROSS_ID) + "\n"
        "  Xp="        + to_string(cfg.Xp)       + "\n"
        "  Mp="        + to_string(cfg.Mp)        + "\n"
        "  turSize="   + to_string(cfg.turSize)   + "\n"
        "  REPAIRp="   + to_string(cfg.REPAIRp)  + "\n"
        "  OPTp="      + to_string(cfg.OPTp)     + "\n"
        "  REDISTp="   + to_string(cfg.REDISTp)  + "\n";
    cout << final_line;
    summary << final_line;
    summary.close();
    return cfg;
}

// ─── Final comparison ────────────────────────────────────────────────────────

const int STAG_GENS = 10000;   // stop if no improvement for this many generations
const int MAX_EVALS = 1000000; // hard cap per run

const Instance CMP_INSTANCES[] = {
    {"./problems/solomon-100/c101.txt",  100, "c101"},
    {"./problems/solomon-100/c102.txt",  100, "c102"},
    {"./problems/solomon-100/c201.txt",  100, "c201"},
    {"./problems/solomon-100/c202.txt",  100, "c202"},
    {"./problems/solomon-100/r101.txt",  100, "r101"},
    {"./problems/solomon-100/r102.txt",  100, "r102"},
    {"./problems/solomon-100/r201.txt",  100, "r201"},
    {"./problems/solomon-100/r202.txt",  100, "r202"},
    {"./problems/solomon-100/rc101.txt", 100, "rc101"},
    {"./problems/solomon-100/rc102.txt", 100, "rc102"},
    {"./problems/solomon-100/rc201.txt", 100, "rc201"},
    {"./problems/solomon-100/rc202.txt", 100, "rc202"},
};
const int N_CMP = 12;

// Best configs found by tuning (from out/v3/tuning_log.txt)
EvoConfig best_ops_on() {
    EvoConfig c;
    c.popSize = 50; c.CROSS_ID = CrossOps::OX_ID;
    c.Xp = 75; c.Mp = 25; c.turSize = 2; c.elitesNum = 1;
    c.REPAIRp = 30; c.OPTp = 10; c.REDISTp = 100;
    return c;
}
EvoConfig best_ops_off() {
    EvoConfig c;
    c.popSize = 50; c.CROSS_ID = CrossOps::OX_ID;
    c.Xp = 75; c.Mp = 25; c.turSize = 2; c.elitesNum = 1;
    c.REPAIRp = 0; c.OPTp = 0; c.REDISTp = 0;
    return c;
}

// Runs CMP_RUNS stagnation-based trials on one instance sequentially.
double cmp_instance(const Instance& inst, const EvoConfig& cfg,
                    const string& label, const string& out_dir) {
    Problem* problem = new Problem(inst.path, inst.size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = 0.01;

    double sum = 0;
    for (int r = 0; r < CMP_RUNS; r++) {
        string csv = out_dir + "/" + inst.name + "_" + label + "_" + to_string(r) + ".csv";
        sum += run_one_stagnation(problem, cfg, csv, STAG_GENS, MAX_EVALS);
    }
    delete problem;
    double avg = sum / CMP_RUNS;
    cout << "  " << label << "  " << inst.name
         << "  avg=" << fixed << setprecision(1) << avg << endl;
    return avg;
}

void run_comparison() {
    const string OUT = "out/compare";
    const string ON_DIR  = OUT + "/on";
    const string OFF_DIR = OUT + "/off";
    for (auto& d : {OUT, ON_DIR, OFF_DIR})
        system(("mkdir -p " + d).c_str());

    ofstream summary(OUT + "/summary.txt");
    summary << "FINAL COMPARISON\n"
            << "Stagnation: " << STAG_GENS << " gens  Max evals: " << MAX_EVALS
            << "  Runs per instance: " << CMP_RUNS << "\n\n";

    auto run_config = [&](const string& label, const EvoConfig& cfg,
                          const string& dir) {
        cout << "\n=== " << label << " ===" << endl;
        summary << "=== " << label << " ===\n";

        double total = 0;
        for (int i = 0; i < N_CMP; i++) {
            double avg = cmp_instance(CMP_INSTANCES[i], cfg, label, dir);
            string line = "  " + CMP_INSTANCES[i].name
                        + "  avg=" + to_string((int)avg);
            summary << line << "\n";
            total += avg;
        }
        double overall = total / N_CMP;
        string ol = "OVERALL avg=" + to_string((int)overall);
        cout << ol << endl;
        summary << ol << "\n\n";
        return overall;
    };

    double on_avg  = run_config("ops_on",  best_ops_on(),  ON_DIR);
    double off_avg = run_config("ops_off", best_ops_off(), OFF_DIR);

    string verdict = on_avg < off_avg ? "ops_on wins" : "ops_off wins";
    cout << "\n" << verdict
         << "  (on=" << fixed << setprecision(1) << on_avg
         << "  off=" << off_avg << ")" << endl;
    summary << verdict
            << "  (on=" << fixed << setprecision(1) << on_avg
            << "  off=" << off_avg << ")\n";
    summary.close();
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main()
{
    run_comparison();
    return 0;
}
