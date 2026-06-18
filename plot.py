#!/usr/bin/env python3
"""
Visualise CVRPTW tuning experiment results.

Reads CSV files from out/ (format: gen;best;avg;worst).
Writes PNG plots to out/plots/.

Usage:
    python plot.py          # generate all plots
    python plot.py --show   # display interactively instead of saving
"""

import argparse
import sys
from pathlib import Path
from collections import defaultdict

try:
    import numpy as np
    import matplotlib
    matplotlib.use("Agg")       # headless default; overridden by --show
    import matplotlib.pyplot as plt
    import matplotlib.ticker as mticker
except ImportError:
    sys.exit("Required: pip install matplotlib numpy")

# ─── Config ──────────────────────────────────────────────────────────────────

CSV_DIR  = Path("out")
PLOT_DIR = Path("out/plots")
N_RUNS   = 5
INSTANCES = ["C101", "R101", "RC101"]

# Series colours (consistent across all plots)
C = {
    "ops_on_best":  "#1f77b4",
    "ops_on_avg":   "#aec7e8",
    "ops_off_best": "#d62728",
    "ops_off_avg":  "#f4a582",
}

INSTANCE_COLORS = ["#1f77b4", "#2ca02c", "#d62728"]   # blue / green / red

# ─── CSV parsing ─────────────────────────────────────────────────────────────

def parse_csv(path: Path) -> dict:
    """Return {gen, best, avg, worst} lists from a CSV file."""
    data = {"gen": [], "best": [], "avg": [], "worst": []}
    with open(path) as f:
        for line in f:
            parts = line.strip().split(";")
            if len(parts) < 4:
                continue
            try:
                data["gen"].append(int(parts[0]))
                data["best"].append(float(parts[1]))
                data["avg"].append(float(parts[2]))
                data["worst"].append(float(parts[3]))
            except ValueError:
                continue
    return data


def load_runs(instance: str, tag: str) -> list[dict]:
    runs = []
    for r in range(N_RUNS):
        p = CSV_DIR / f"{instance}_{tag}_{r}.csv"
        if p.exists():
            runs.append(parse_csv(p))
    return runs


def mean_curve(runs: list[dict], key: str):
    """Average a metric across runs, return (gens, mean_values)."""
    if not runs:
        return [], []
    gen = runs[0]["gen"]
    arr = [[r[key][i] for i in range(len(gen))] for r in runs]
    return gen, list(np.mean(arr, axis=0))


def final_mean(runs: list[dict], key: str = "best") -> float:
    if not runs:
        return float("nan")
    return float(np.mean([r[key][-1] for r in runs]))


def final_best(runs: list[dict]) -> float:
    if not runs:
        return float("nan")
    return float(min(r["best"][-1] for r in runs))


# ─── Helpers ─────────────────────────────────────────────────────────────────

def savefig(fig, name: str, show: bool):
    if show:
        plt.show()
    else:
        PLOT_DIR.mkdir(parents=True, exist_ok=True)
        out = PLOT_DIR / f"{name}.png"
        fig.savefig(out, dpi=150, bbox_inches="tight")
        print(f"  saved {out}")
    plt.close(fig)


def _bar_chart(ax, labels, values_by_instance, ylabel, title):
    """Grouped bar chart: x=labels, one group of bars per instance."""
    x = np.arange(len(labels))
    n = len(INSTANCES)
    w = 0.22
    offsets = np.linspace(-(n-1)*w/2, (n-1)*w/2, n)
    for i, (inst, color) in enumerate(zip(INSTANCES, INSTANCE_COLORS)):
        ax.bar(x + offsets[i], values_by_instance[inst], w,
               label=inst, color=color, alpha=0.85)
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.legend()
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{v:.0f}"))
    ax.grid(axis="y", linestyle="--", alpha=0.4)


# ─── Plot functions ───────────────────────────────────────────────────────────

def plot_phase1(show: bool):
    """Bar chart: ops_on vs ops_off, avg final cost, per instance."""
    fig, ax = plt.subplots(figsize=(7, 5))
    x = np.arange(len(INSTANCES))
    w = 0.35
    off_vals = [final_mean(load_runs(inst, "ops_off")) for inst in INSTANCES]
    on_vals  = [final_mean(load_runs(inst, "ops_on"))  for inst in INSTANCES]

    ax.bar(x - w/2, off_vals, w, label="Bez operatorów",  color=C["ops_off_best"], alpha=0.85)
    ax.bar(x + w/2, on_vals,  w, label="Z operatorami",   color=C["ops_on_best"],  alpha=0.85)

    for xi, (a, b) in enumerate(zip(off_vals, on_vals)):
        gain = 100 * (a - b) / a
        ax.annotate(f"−{gain:.1f}%", xy=(xi + w/2, b), ha="center", va="bottom",
                    fontsize=8, color="#1f77b4")

    ax.set_xticks(x); ax.set_xticklabels(INSTANCES)
    ax.set_ylabel("Śr. koszt (N=5 uruchomień)")
    ax.set_title("Faza 1: wpływ operatorów niestandardowych")
    ax.legend(); ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()
    savefig(fig, "phase1_operators", show)


def plot_phase2(show: bool):
    """Bar chart: population size vs avg final cost."""
    pop_labels = ["10", "20", "50", "100"]
    pop_tags   = ["pop10", "pop20", "pop50", "pop100"]
    values = {inst: [final_mean(load_runs(inst, t)) for t in pop_tags]
              for inst in INSTANCES}
    fig, ax = plt.subplots(figsize=(7, 5))
    _bar_chart(ax, pop_labels, values,
               "Śr. koszt (N=5 uruchomień)",
               "Faza 2: rozmiar populacji (z operatorami)")
    ax.set_xlabel("Rozmiar populacji")
    fig.tight_layout()
    savefig(fig, "phase2_popsize", show)


def plot_phase3(show: bool):
    """Bar chart: Xp vs avg final cost."""
    xp_labels = ["50", "75", "90"]
    xp_tags   = ["xp50", "xp75", "xp90"]
    values = {inst: [final_mean(load_runs(inst, t)) for t in xp_tags]
              for inst in INSTANCES}
    fig, ax = plt.subplots(figsize=(7, 5))
    _bar_chart(ax, xp_labels, values,
               "Śr. koszt (N=5 uruchomień)",
               "Faza 3: prawdopodobieństwo krzyżowania Xp")
    ax.set_xlabel("Xp (%)")
    fig.tight_layout()
    savefig(fig, "phase3_xp", show)


def plot_phase4(show: bool):
    """Bar chart: Mp vs avg final cost."""
    mp_labels = ["10", "25", "40"]
    mp_tags   = ["mp10", "mp25", "mp40"]
    values = {inst: [final_mean(load_runs(inst, t)) for t in mp_tags]
              for inst in INSTANCES}
    fig, ax = plt.subplots(figsize=(7, 5))
    _bar_chart(ax, mp_labels, values,
               "Śr. koszt (N=5 uruchomień)",
               "Faza 4: prawdopodobieństwo mutacji Mp")
    ax.set_xlabel("Mp (%)")
    fig.tight_layout()
    savefig(fig, "phase4_mp", show)


def plot_phase5_convergence(show: bool):
    """Convergence curves (mean best + mean avg) for ops_on vs ops_off, per instance."""
    for inst in INSTANCES:
        fig, ax = plt.subplots(figsize=(10, 5))

        for tag, label, bc, ac in [
            ("final_ops_off", "bez operatorów", C["ops_off_best"], C["ops_off_avg"]),
            ("final_ops_on",  "z operatorami",  C["ops_on_best"],  C["ops_on_avg"]),
        ]:
            runs = load_runs(inst, tag)
            gen, best = mean_curve(runs, "best")
            _,   avg  = mean_curve(runs, "avg")
            if not gen:
                print(f"  [WARN] no data for {inst}/{tag}")
                continue
            ax.plot(gen, best, color=bc, linewidth=1.8, label=f"{label} – najlepsza")
            ax.plot(gen, avg,  color=ac, linewidth=1.2, linestyle="--",
                    label=f"{label} – średnia")

        ax.set_xlabel("Nr generacji")
        ax.set_ylabel("Koszt rozwiązania")
        ax.set_title(f"Faza 5: przebieg EA – {inst} (budżet 200 000)")
        ax.legend(fontsize=9); ax.grid(linestyle="--", alpha=0.4)
        fig.tight_layout()
        savefig(fig, f"phase5_{inst.lower()}", show)


def plot_phase5_bar(show: bool):
    """Bar chart: final ON vs OFF for each instance (full-budget run)."""
    fig, ax = plt.subplots(figsize=(7, 5))
    x = np.arange(len(INSTANCES))
    w = 0.35
    off_vals = [final_mean(load_runs(inst, "final_ops_off")) for inst in INSTANCES]
    on_vals  = [final_mean(load_runs(inst, "final_ops_on"))  for inst in INSTANCES]

    ax.bar(x - w/2, off_vals, w, label="Bez operatorów",  color=C["ops_off_best"], alpha=0.85)
    ax.bar(x + w/2, on_vals,  w, label="Z operatorami",   color=C["ops_on_best"],  alpha=0.85)

    for xi, (a, b) in enumerate(zip(off_vals, on_vals)):
        gain = 100 * (a - b) / a
        ax.annotate(f"−{gain:.1f}%", xy=(xi + w/2, b), ha="center", va="bottom",
                    fontsize=8, color="#1f77b4")

    ax.set_xticks(x); ax.set_xticklabels(INSTANCES)
    ax.set_ylabel("Śr. koszt (N=5 uruchomień)")
    ax.set_title("Faza 5: operatory ON vs OFF (pełny budżet 200 000)")
    ax.legend(); ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()
    savefig(fig, "phase5_bar", show)


# ─── Entry point ─────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--show", action="store_true",
                        help="display plots interactively instead of saving PNGs")
    args = parser.parse_args()

    if args.show:
        matplotlib.use("TkAgg")   # switch to interactive backend

    print("Generating plots...")
    plot_phase1(args.show)
    plot_phase2(args.show)
    plot_phase3(args.show)
    plot_phase4(args.show)
    plot_phase5_convergence(args.show)
    plot_phase5_bar(args.show)
    if not args.show:
        print(f"Done. All plots in {PLOT_DIR}/")


if __name__ == "__main__":
    main()
