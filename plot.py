#!/usr/bin/env python3
"""
Generate all plots for the CVRPTW report.
Reads:
  out/v3/on/   - ops_on tuning CSVs
  out/v3/off/  - ops_off tuning CSVs
  out/compare/ - final comparison CSVs
Writes PNGs to out/plots/.

Usage: python plot.py
"""

import sys
from pathlib import Path
import numpy as np

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.ticker as mticker
except ImportError:
    sys.exit("pip install matplotlib numpy")

PLOT_DIR = Path("out/plots")
N_RUNS   = 5
TUNE_INSTANCES = ["c101", "r101", "rc101"]

COL_ON  = "#1f77b4"
COL_OFF = "#d62728"
COL_ON_L  = "#aec7e8"
COL_OFF_L = "#f4a582"
INST_COLS = ["#1f77b4", "#2ca02c", "#d62728"]

# ---------- CSV helpers -------------------------------------------------------

def parse_csv(path):
    gen, best, avg, worst = [], [], [], []
    with open(path) as f:
        for line in f:
            p = line.strip().split(";")
            if len(p) < 4:
                continue
            try:
                gen.append(int(p[0]))
                best.append(float(p[1]))
                avg.append(float(p[2]))
                worst.append(float(p[3]))
            except ValueError:
                continue
    return {"gen": gen, "best": best, "avg": avg, "worst": worst}


def load_runs(directory, inst, tag):
    runs = []
    for r in range(N_RUNS):
        p = Path(directory) / f"{inst}_{tag}_{r}.csv"
        if p.exists():
            runs.append(parse_csv(p))
    return runs


def final_avg(runs):
    if not runs:
        return float("nan")
    return float(np.mean([r["best"][-1] for r in runs]))


def final_std(runs):
    if not runs or len(runs) < 2:
        return float("nan")
    return float(np.std([r["best"][-1] for r in runs], ddof=1))


def final_best_val(runs):
    if not runs:
        return float("nan")
    return float(min(r["best"][-1] for r in runs))


def mean_curve(runs, key="best"):
    if not runs:
        return [], []
    min_len = min(len(r[key]) for r in runs)
    arr = [r[key][:min_len] for r in runs]
    gen = runs[0]["gen"][:min_len]
    return gen, list(np.mean(arr, axis=0))

# ---------- Save helper -------------------------------------------------------

def save(fig, name):
    PLOT_DIR.mkdir(parents=True, exist_ok=True)
    out = PLOT_DIR / f"{name}.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    print(f"  {out}")
    plt.close(fig)

# ---------- Bar chart helper --------------------------------------------------

def grouped_bar(ax, variants, label_map, values_dict, title, xlabel, ylabel="Sredni koszt finalny (N=5)"):
    """values_dict: {inst: [val per variant]}"""
    x = np.arange(len(variants))
    n = len(TUNE_INSTANCES)
    w = 0.22
    offsets = np.linspace(-(n-1)*w/2, (n-1)*w/2, n)
    for i, (inst, col) in enumerate(zip(TUNE_INSTANCES, INST_COLS)):
        vals = values_dict[inst]
        bars = ax.bar(x + offsets[i], vals, w, label=inst.upper(), color=col, alpha=0.85)
        for bar, v in zip(bars, vals):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 5,
                    f"{v:.0f}", ha="center", va="bottom", fontsize=6.5)
    ax.set_xticks(x)
    ax.set_xticklabels([label_map.get(v, v) for v in variants], fontsize=9)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.legend(fontsize=8)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{v:.0f}"))


def single_bar(ax, variants, label_map, values, colors, title, xlabel):
    x = np.arange(len(variants))
    bars = ax.bar(x, values, 0.5, color=colors, alpha=0.85)
    for bar, v in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 3,
                f"{v:.0f}", ha="center", va="bottom", fontsize=8)
    ax.set_xticks(x)
    ax.set_xticklabels([label_map.get(v, v) for v in variants], fontsize=9)
    ax.set_xlabel(xlabel)
    ax.set_ylabel("Sredni koszt finalny (N=5 x 3 instancje)")
    ax.set_title(title)
    ax.grid(axis="y", linestyle="--", alpha=0.4)


# =============================================================================
# TUNING PLOTS
# =============================================================================

def tuning_phase(name, directory, variants, label_map, xlabel, title, filename):
    """One grouped bar chart per tuning phase."""
    values = {inst: [final_avg(load_runs(directory, inst, v)) for v in variants]
              for inst in TUNE_INSTANCES}
    fig, ax = plt.subplots(figsize=(8, 5))
    grouped_bar(ax, variants, label_map, values, title, xlabel)
    fig.tight_layout()
    save(fig, filename)


def tuning_phase_overall(name, directory, variants, label_map, xlabel, title, filename):
    """Single bar chart: overall avg across 3 instances."""
    overall = []
    for v in variants:
        s = sum(final_avg(load_runs(directory, inst, v)) for inst in TUNE_INSTANCES)
        overall.append(s / len(TUNE_INSTANCES))
    # highlight winner
    mn = min(overall)
    colors = [COL_ON if x == mn else "#aaaaaa" for x in overall]
    fig, ax = plt.subplots(figsize=(7, 4))
    single_bar(ax, variants, label_map, overall, colors, title, xlabel)
    fig.tight_layout()
    save(fig, filename)


# ---------- ops_on pipeline ---------------------------------------------------

def plot_tuning_on():
    d = "out/v3/on"

    tuning_phase("popsize", d,
                 ["pop10","pop20","pop50","pop100"], {"pop10":"10","pop20":"20","pop50":"50","pop100":"100"},
                 "Rozmiar populacji",
                 "Strojenie (ops_on) - faza 1: rozmiar populacji",
                 "on_phase1_pop")

    tuning_phase("cross", d,
                 ["cross_OX","cross_PMX","cross_CX"], {"cross_OX":"OX","cross_PMX":"PMX","cross_CX":"CX"},
                 "Operator krzyzowania",
                 "Strojenie (ops_on) - faza 2: operator krzyzowania",
                 "on_phase2_cross")

    tuning_phase("xp", d,
                 ["xp50","xp75","xp90"], {"xp50":"50%","xp75":"75%","xp90":"90%"},
                 "Xp (%)",
                 "Strojenie (ops_on) - faza 3: prawdopodobienstwo krzyzowania",
                 "on_phase3_xp")

    tuning_phase("mp", d,
                 ["mp10","mp25","mp40"], {"mp10":"10%","mp25":"25%","mp40":"40%"},
                 "Mp (%)",
                 "Strojenie (ops_on) - faza 4: prawdopodobienstwo mutacji",
                 "on_phase4_mp")

    tuning_phase("tur", d,
                 ["tur2","tur3","tur5"], {"tur2":"2","tur3":"3","tur5":"5"},
                 "Rozmiar turnieju",
                 "Strojenie (ops_on) - faza 5: rozmiar turnieju",
                 "on_phase5_tur")

    tuning_phase("repair", d,
                 ["repair30","repair70","repair100"], {"repair30":"30%","repair70":"70%","repair100":"100%"},
                 "REPAIRp (%)",
                 "Strojenie (ops_on) - faza 6: prawdopodobienstwo Repair",
                 "on_phase6_repair")

    tuning_phase("opt", d,
                 ["opt10","opt30","opt60"], {"opt10":"10%","opt30":"30%","opt60":"60%"},
                 "OPTp (%)",
                 "Strojenie (ops_on) - faza 7: prawdopodobienstwo OptimizeTracks",
                 "on_phase7_opt")

    tuning_phase("redist", d,
                 ["redist30","redist80","redist100"], {"redist30":"30%","redist80":"80%","redist100":"100%"},
                 "REDISTp (%)",
                 "Strojenie (ops_on) - faza 8: prawdopodobienstwo RedistributeLocations",
                 "on_phase8_redist")


# ---------- ops_off pipeline --------------------------------------------------

def plot_tuning_off():
    d = "out/v3/off"

    tuning_phase("popsize", d,
                 ["pop10","pop20","pop50","pop100"], {"pop10":"10","pop20":"20","pop50":"50","pop100":"100"},
                 "Rozmiar populacji",
                 "Strojenie (ops_off) - faza 1: rozmiar populacji",
                 "off_phase1_pop")

    tuning_phase("cross", d,
                 ["cross_OX","cross_PMX","cross_CX"], {"cross_OX":"OX","cross_PMX":"PMX","cross_CX":"CX"},
                 "Operator krzyzowania",
                 "Strojenie (ops_off) - faza 2: operator krzyzowania",
                 "off_phase2_cross")

    tuning_phase("xp", d,
                 ["xp50","xp75","xp90"], {"xp50":"50%","xp75":"75%","xp90":"90%"},
                 "Xp (%)",
                 "Strojenie (ops_off) - faza 3: prawdopodobienstwo krzyzowania",
                 "off_phase3_xp")

    tuning_phase("mp", d,
                 ["mp10","mp25","mp40"], {"mp10":"10%","mp25":"25%","mp40":"40%"},
                 "Mp (%)",
                 "Strojenie (ops_off) - faza 4: prawdopodobienstwo mutacji",
                 "off_phase4_mp")

    tuning_phase("tur", d,
                 ["tur2","tur3","tur5"], {"tur2":"2","tur3":"3","tur5":"5"},
                 "Rozmiar turnieju",
                 "Strojenie (ops_off) - faza 5: rozmiar turnieju",
                 "off_phase5_tur")


# =============================================================================
# COMPARISON PLOTS
# =============================================================================

CMP_INSTANCES = ["c101","c102","c201","c202","r101","r102","r201","r202",
                 "rc101","rc102","rc201","rc202"]
CMP_LABELS    = ["C101","C102","C201","C202","R101","R102","R201","R202",
                 "RC101","RC102","RC201","RC202"]

def plot_compare_bar():
    on_vals  = [final_avg(load_runs("out/compare/on",  inst, "ops_on"))  for inst in CMP_INSTANCES]
    off_vals = [final_avg(load_runs("out/compare/off", inst, "ops_off")) for inst in CMP_INSTANCES]

    x = np.arange(len(CMP_INSTANCES))
    w = 0.38
    fig, ax = plt.subplots(figsize=(14, 6))
    b1 = ax.bar(x - w/2, off_vals, w, label="EA bazowy (ops_off)", color=COL_OFF, alpha=0.85)
    b2 = ax.bar(x + w/2, on_vals,  w, label="EA z operatorami (ops_on)", color=COL_ON, alpha=0.85)

    for xi, (a, b) in enumerate(zip(off_vals, on_vals)):
        diff = a - b
        sign = "-" if diff >= 0 else "+"
        ax.annotate(f"{sign}{abs(diff):.0f}",
                    xy=(xi + w/2, min(a, b) - 15), ha="center", va="top",
                    fontsize=7, color="black")

    on_avg  = np.mean(on_vals)
    off_avg = np.mean(off_vals)
    ax.axhline(on_avg,  color=COL_ON,  linestyle="--", linewidth=1.2,
               label=f"ops_on srednia ({on_avg:.0f})")
    ax.axhline(off_avg, color=COL_OFF, linestyle="--", linewidth=1.2,
               label=f"ops_off srednia ({off_avg:.0f})")

    ax.set_xticks(x)
    ax.set_xticklabels(CMP_LABELS, fontsize=9)
    ax.set_ylabel("Sredni koszt finalny (N=5 uruchomien)")
    ax.set_title("Porownanie koncowe: EA z operatorami vs EA bazowy (12 instancji Solomona)")
    ax.legend(fontsize=9)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()
    save(fig, "compare_bar")


def plot_compare_convergence(inst_name):
    """Convergence curves: ops_on vs ops_off for one instance from compare/."""
    on_runs  = load_runs("out/compare/on",  inst_name, "ops_on")
    off_runs = load_runs("out/compare/off", inst_name, "ops_off")

    fig, ax = plt.subplots(figsize=(10, 5))

    if off_runs:
        gen, best = mean_curve(off_runs, "best")
        _,   avg  = mean_curve(off_runs, "avg")
        ax.plot(gen, best, color=COL_OFF,   lw=1.8, label="ops_off - najlepsza")
        ax.plot(gen, avg,  color=COL_OFF_L, lw=1.2, ls="--", label="ops_off - srednia")

    if on_runs:
        gen, best = mean_curve(on_runs, "best")
        _,   avg  = mean_curve(on_runs, "avg")
        ax.plot(gen, best, color=COL_ON,   lw=1.8, label="ops_on - najlepsza")
        ax.plot(gen, avg,  color=COL_ON_L, lw=1.2, ls="--", label="ops_on - srednia")

    ax.set_xlabel("Nr generacji")
    ax.set_ylabel("Koszt rozwiazania")
    ax.set_title(f"Przebieg EA - instancja {inst_name.upper()} (kryterium stagnacji 10 000 gen)")
    ax.legend(fontsize=9)
    ax.grid(linestyle="--", alpha=0.4)
    fig.tight_layout()
    save(fig, f"compare_conv_{inst_name}")


def write_compare_csv():
    """Write out/compare/results.csv with avg;std;best per instance per config."""
    import csv
    out = Path("out/compare/results.csv")
    with open(out, "w", newline="") as f:
        w = csv.writer(f, delimiter=";")
        w.writerow(["instance", "config", "avg", "std", "best"])
        for inst in CMP_INSTANCES:
            for config, label, d in [
                ("ops_on",  "ops_on",  "out/compare/on"),
                ("ops_off", "ops_off", "out/compare/off"),
            ]:
                runs = load_runs(d, inst, label)
                w.writerow([inst, config,
                             f"{final_avg(runs):.2f}",
                             f"{final_std(runs):.2f}",
                             f"{final_best_val(runs):.2f}"])
    print(f"  {out}")


def plot_compare_grouped_by_family():
    """Bar chart grouped by family (C1, C2, R1, R2, RC1, RC2)."""
    families = {
        "C1":  ["c101","c102"],
        "C2":  ["c201","c202"],
        "R1":  ["r101","r102"],
        "R2":  ["r201","r202"],
        "RC1": ["rc101","rc102"],
        "RC2": ["rc201","rc202"],
    }
    fam_names = list(families.keys())
    on_fam  = [np.mean([final_avg(load_runs("out/compare/on",  i, "ops_on"))  for i in v]) for v in families.values()]
    off_fam = [np.mean([final_avg(load_runs("out/compare/off", i, "ops_off")) for i in v]) for v in families.values()]

    x = np.arange(len(fam_names))
    w = 0.38
    fig, ax = plt.subplots(figsize=(9, 5))
    ax.bar(x - w/2, off_fam, w, label="EA bazowy",         color=COL_OFF, alpha=0.85)
    ax.bar(x + w/2, on_fam,  w, label="EA z operatorami",  color=COL_ON,  alpha=0.85)

    for xi, (a, b) in enumerate(zip(off_fam, on_fam)):
        gain = 100 * (a - b) / a
        sign = "-" if gain >= 0 else "+"
        ax.annotate(f"{sign}{abs(gain):.1f}%",
                    xy=(xi + w/2, b + 3), ha="center", va="bottom",
                    fontsize=8, color="black")

    ax.set_xticks(x)
    ax.set_xticklabels(fam_names)
    ax.set_ylabel("Sredni koszt (srednia 2 instancji x 5 uruchomien)")
    ax.set_title("Porownanie koncowe wg rodziny instancji Solomona")
    ax.legend()
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()
    save(fig, "compare_by_family")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("Generating tuning plots (ops_on)...")
    plot_tuning_on()
    print("Generating tuning plots (ops_off)...")
    plot_tuning_off()
    print("Generating comparison plots...")
    write_compare_csv()
    plot_compare_bar()
    plot_compare_grouped_by_family()
    for inst in ["c101", "r101", "rc101"]:
        plot_compare_convergence(inst)
    print(f"Done. All plots in {PLOT_DIR}/")


if __name__ == "__main__":
    main()
