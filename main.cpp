#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
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

const int N_RUNS   = 5;             // runs per config during tuning
const int CMP_RUNS = 10;            // runs per config during final comparison

const int TUNE_STAG_GENS = 5000;       // tuning stop: no improvement for this many gens
const int TUNE_MAX_EVALS = 10'000'000; // tuning hard eval cap (safety net)

const int CMP_STAG_GENS  = 10000;
const int CMP_MAX_EVALS  = 1'000'000;

// Penalty per time unit of late arrival. 0.01 was too small -- algorithm ignored TW.
// At 1.0 a TW violation costs the same as 1 distance unit, making Repair actually useful.
const double LATE_PENALTY = 1.0;

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
    int REPAIRp   = 0;
    int OPTp      = 0;
    int REDISTp   = 0;
    int REDIST_TRIES                = 8;
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
    evo->REDIST_TRIES                = cfg.REDIST_TRIES;
    evo->OPT_TRACK_MAX_LOCATION_COUNT = cfg.OPT_TRACK_MAX_LOCATION_COUNT;
    return evo;
}

// ─── Stats helpers ───────────────────────────────────────────────────────────

struct RunStats { double avg; double std_dev; };

double sample_std(const vector<double>& v) {
    if (v.size() < 2) return 0.0;
    double m = 0; for (double x : v) m += x; m /= v.size();
    double sq = 0; for (double x : v) sq += (x - m) * (x - m);
    return sqrt(sq / (v.size() - 1));
}

// ─── Core runs ───────────────────────────────────────────────────────────────

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
    int max_gens   = max_evals / cfg.popSize;

    for (int i = 1; i <= max_gens && no_improve < stag_gens; i++) {
        evo->Evolve(); evo->Eval();
        Solution* b = evo->GetBest(); Solution* w = evo->GetWorst();
        if (b->eval < best_eval - 1e-9) {
            best_eval  = b->eval;
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

// N_RUNS stagnation-based trials on one tuning instance.
RunStats run_instance(const Instance& inst, const EvoConfig& cfg,
                      const string& tag, const string& out_dir) {
    Problem* problem = new Problem(inst.path, inst.size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = LATE_PENALTY;

    vector<double> results;
    for (int r = 0; r < N_RUNS; r++) {
        string csv = out_dir + "/" + inst.name + "_" + tag + "_" + to_string(r) + ".csv";
        results.push_back(run_one_stagnation(problem, cfg, csv, TUNE_STAG_GENS, TUNE_MAX_EVALS));
    }
    delete problem;
    double m = 0; for (double x : results) m += x; m /= N_RUNS;
    return {m, sample_std(results)};
}

// ─── Phase runner ────────────────────────────────────────────────────────────

struct Variant { string tag; EvoConfig cfg; };

struct PhaseResult {
    string    winner_tag;
    EvoConfig winner_cfg;
    double    inst_avgs[3];
    double    inst_stds[3];
    double    overall_avg;
};

PhaseResult run_phase(const string& phase_name,
                      const vector<Variant>& variants,
                      const string& out_dir,
                      ofstream& summary) {
    cout << "\n--- " << phase_name << " ---" << endl;
    summary << "\n--- " << phase_name << " ---\n";

    PhaseResult best;
    best.overall_avg = 1e18;

    for (auto& v : variants) {
        double inst_avgs[3], inst_stds[3], total = 0;
        for (int i = 0; i < N_INSTANCES; i++) {
            RunStats s = run_instance(INSTANCES[i], v.cfg, v.tag, out_dir);
            inst_avgs[i] = s.avg;
            inst_stds[i] = s.std_dev;
            total += s.avg;
        }
        double overall = total / N_INSTANCES;

        auto fmt = [](double avg, double std) {
            return to_string((int)avg) + "(+-" + to_string((int)std) + ")";
        };
        string line = "  " + v.tag
            + "  c101="  + fmt(inst_avgs[0], inst_stds[0])
            + "  r101="  + fmt(inst_avgs[1], inst_stds[1])
            + "  rc101=" + fmt(inst_avgs[2], inst_stds[2])
            + "  OVERALL=" + to_string((int)overall);
        if (overall < best.overall_avg) {
            line += "  <-- best";
            best.winner_tag = v.tag;
            best.winner_cfg = v.cfg;
            for (int i = 0; i < N_INSTANCES; i++) {
                best.inst_avgs[i] = inst_avgs[i];
                best.inst_stds[i] = inst_stds[i];
            }
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

EvoConfig tune_pipeline(EvoConfig base, bool with_ops, const string& out_dir) {
    string label = with_ops ? "OPS_ON" : "OPS_OFF";
    ofstream summary(out_dir + "/summary_" + label + ".txt");
    cout << "\n\n========================================" << endl;
    cout << "  PIPELINE: " << label << endl;
    cout << "========================================" << endl;
    summary << "PIPELINE: " << label
            << "\nStag gens: " << TUNE_STAG_GENS
            << "  Max evals: " << TUNE_MAX_EVALS
            << "  Runs: " << N_RUNS << "x3 instances\n";

    EvoConfig cfg = base;

    // Phase 1: population size -- wide range to explore diversity vs speed
    {
        vector<Variant> vs;
        for (int pop : {50, 500, 5000, 10000}) {
            EvoConfig c = cfg; c.popSize = pop;
            vs.push_back({"pop" + to_string(pop), c});
        }
        cfg = run_phase("Phase 1: popSize", vs, out_dir, summary).winner_cfg;
    }

    // Phase 2: crossover operator
    {
        vector<Variant> vs;
        for (auto [name, id] : vector<pair<string,int>>{
                {"OX", CrossOps::OX_ID}, {"PMX", CrossOps::PMX_ID}, {"CX", CrossOps::CX_ID}}) {
            EvoConfig c = cfg; c.CROSS_ID = id;
            vs.push_back({"cross_" + name, c});
        }
        cfg = run_phase("Phase 2: crossover", vs, out_dir, summary).winner_cfg;
    }

    // Phase 3: mutation operator
    {
        vector<Variant> vs;
        for (auto [name, id] : vector<pair<string,int>>{
                {"SWAP", MutationOps::SWAP_ID}, {"INVERSE", MutationOps::INVERSE_ID}}) {
            EvoConfig c = cfg; c.MUT_ID = id;
            vs.push_back({"mut_" + name, c});
        }
        cfg = run_phase("Phase 3: mutation op", vs, out_dir, summary).winner_cfg;
    }

    // Phase 4: crossover probability
    {
        vector<Variant> vs;
        for (int xp : {25, 50, 75, 95}) {
            EvoConfig c = cfg; c.Xp = xp;
            vs.push_back({"xp" + to_string(xp), c});
        }
        cfg = run_phase("Phase 4: Xp", vs, out_dir, summary).winner_cfg;
    }

    // Phase 5: mutation probability
    {
        vector<Variant> vs;
        for (int mp : {5, 25, 50, 75}) {
            EvoConfig c = cfg; c.Mp = mp;
            vs.push_back({"mp" + to_string(mp), c});
        }
        cfg = run_phase("Phase 5: Mp", vs, out_dir, summary).winner_cfg;
    }

    // Phase 6: tournament size (higher = more selection pressure = less diversity)
    {
        vector<Variant> vs;
        for (int ts : {2, 3, 5, 7}) {
            EvoConfig c = cfg; c.turSize = ts;
            vs.push_back({"tur" + to_string(ts), c});
        }
        cfg = run_phase("Phase 6: turSize", vs, out_dir, summary).winner_cfg;
    }

    if (with_ops) {
        // Phase 7: REPAIRp
        {
            vector<Variant> vs;
            for (int rp : {0, 10, 50, 100}) {
                EvoConfig c = cfg; c.REPAIRp = rp;
                vs.push_back({"repair" + to_string(rp), c});
            }
            cfg = run_phase("Phase 7: REPAIRp", vs, out_dir, summary).winner_cfg;
        }
        // Phase 8: OPTp
        {
            vector<Variant> vs;
            for (int op : {0, 5, 25, 50}) {
                EvoConfig c = cfg; c.OPTp = op;
                vs.push_back({"opt" + to_string(op), c});
            }
            cfg = run_phase("Phase 8: OPTp", vs, out_dir, summary).winner_cfg;
        }
        // Phase 9: REDISTp
        {
            vector<Variant> vs;
            for (int rd : {0, 25, 75, 100}) {
                EvoConfig c = cfg; c.REDISTp = rd;
                vs.push_back({"redist" + to_string(rd), c});
            }
            cfg = run_phase("Phase 9: REDISTp", vs, out_dir, summary).winner_cfg;
        }
    }

    auto cross_name = [](int id) -> string {
        if (id == CrossOps::OX_ID)  return "OX";
        if (id == CrossOps::PMX_ID) return "PMX";
        return "CX";
    };
    string final_line =
        "\nFINAL BEST CONFIG (" + label + "):\n"
        "  popSize="  + to_string(cfg.popSize)   + "\n"
        "  CROSS="    + cross_name(cfg.CROSS_ID)  + "\n"
        "  Xp="       + to_string(cfg.Xp)        + "\n"
        "  Mp="       + to_string(cfg.Mp)         + "\n"
        "  turSize="  + to_string(cfg.turSize)    + "\n"
        "  REPAIRp="  + to_string(cfg.REPAIRp)   + "\n"
        "  OPTp="     + to_string(cfg.OPTp)      + "\n"
        "  REDISTp="  + to_string(cfg.REDISTp)   + "\n";
    cout << final_line;
    summary << final_line;
    summary.close();
    return cfg;
}

// ─── Final comparison ────────────────────────────────────────────────────────

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

RunStats cmp_instance(const Instance& inst, const EvoConfig& cfg,
                      const string& label, const string& out_dir) {
    Problem* problem = new Problem(inst.path, inst.size);
    problem->EARLY_ARRIVAL_PENALTY_MULTIPLAYER = 0;
    problem->LATE_ARRIVAL_PENALTY_MULTIPLAYER  = LATE_PENALTY;

    vector<double> results;
    for (int r = 0; r < CMP_RUNS; r++) {
        string csv = out_dir + "/" + inst.name + "_" + label + "_" + to_string(r) + ".csv";
        results.push_back(run_one_stagnation(problem, cfg, csv, CMP_STAG_GENS, CMP_MAX_EVALS));
    }
    delete problem;
    double m = 0; for (double x : results) m += x; m /= CMP_RUNS;
    double s = sample_std(results);
    cout << "  " << label << "  " << inst.name
         << "  avg=" << fixed << setprecision(1) << m
         << "  std=" << setprecision(1) << s << endl;
    return {m, s};
}

void run_comparison(const EvoConfig& on_cfg, const EvoConfig& off_cfg,
                    const string& out_base) {
    const string ON_DIR  = out_base + "/on";
    const string OFF_DIR = out_base + "/off";
    for (auto& d : {out_base, ON_DIR, OFF_DIR})
        system(("mkdir -p " + d).c_str());

    ofstream summary(out_base + "/summary.txt");
    summary << "FINAL COMPARISON\n"
            << "Stagnation: " << CMP_STAG_GENS << " gens  Max evals: " << CMP_MAX_EVALS
            << "  Runs per instance: " << CMP_RUNS << "\n\n";

    auto run_config = [&](const string& label, const EvoConfig& cfg,
                          const string& dir) {
        cout << "\n=== " << label << " ===" << endl;
        summary << "=== " << label << " ===\n";

        double total = 0;
        for (int i = 0; i < N_CMP; i++) {
            RunStats s = cmp_instance(CMP_INSTANCES[i], cfg, label, dir);
            string line = "  " + CMP_INSTANCES[i].name
                        + "  avg=" + to_string((int)s.avg)
                        + "  std=" + to_string((int)s.std_dev);
            summary << line << "\n";
            total += s.avg;
        }
        double overall = total / N_CMP;
        string ol = "OVERALL avg=" + to_string((int)overall);
        cout << ol << endl;
        summary << ol << "\n\n";
        return overall;
    };

    double on_avg  = run_config("ops_on",  on_cfg,  ON_DIR);
    double off_avg = run_config("ops_off", off_cfg, OFF_DIR);

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
    const string V4      = "out/v4";
    const string ON_DIR  = V4 + "/on";
    const string OFF_DIR = V4 + "/off";
    const string CMP_DIR = V4 + "/compare";
    for (auto& d : {V4, ON_DIR, OFF_DIR, CMP_DIR})
        system(("mkdir -p " + d).c_str());

    EvoConfig on_cfg  = tune_pipeline(ops_on_base(),  true,  ON_DIR);
    EvoConfig off_cfg = tune_pipeline(ops_off_base(), false, OFF_DIR);
    run_comparison(on_cfg, off_cfg, CMP_DIR);
    return 0;
}
