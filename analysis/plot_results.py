"""
Unreal Learning Agents 보상 변형 비교 분석 스크립트.

사용법:
  1. Saved/Logs/RL/ 안의 R1/R2/R3 CSV 파일 경로를 아래 CSV_FILES에 입력
  2. python plot_results.py
  3. 출력: figures/learning_curves.png, figures/success_rate.png, figures/summary_table.csv

요구 패키지: pandas, matplotlib, numpy
  pip install pandas matplotlib numpy
"""

from pathlib import Path
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# ---------- 설정 ----------
PROJECT_ROOT = Path(__file__).resolve().parent.parent
LOG_DIR = PROJECT_ROOT / "Saved" / "Logs" / "RL"
OUT_DIR = Path(__file__).resolve().parent / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

MAX_STEPS = 800              # 환경 MaxSteps (Timeout 판정용)
SMOOTH_WINDOW = 50           # 학습 곡선 이동 평균 윈도우
FINAL_TAIL = 200             # "최종 성공률" 계산 시 마지막 N 에피소드

VARIANT_LABELS = {
    "R1": "R1 - Sparse",
    "R2": "R2 - Dense",
    "R3": "R3 - Dense+Ori+Cost",
}
VARIANT_COLORS = {
    "R1": "#d62728",
    "R2": "#2ca02c",
    "R3": "#1f77b4",
}

# ---------- CSV 자동 매칭 ----------
def find_latest_csv(variant_prefix: str) -> Path | None:
    """주어진 variant 접두사(R1/R2/R3)에 매칭되는 가장 최신 CSV 파일을 반환"""
    pattern = str(LOG_DIR / f"RL_{variant_prefix}_Seed*_*.csv")
    matches = sorted(glob.glob(pattern))
    return Path(matches[-1]) if matches else None

CSV_FILES = {v: find_latest_csv(v) for v in ["R1", "R2", "R3"]}

print("=== Detected CSV files ===")
for v, p in CSV_FILES.items():
    print(f"  {v}: {p if p else '(NOT FOUND)'}")

# ---------- 데이터 로드 + 필터 ----------
def load_clean(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path)
    # 자연 종료(Success 또는 Timeout)만 유지
    df = df[(df["Success"] == 1) | (df["Steps"] >= MAX_STEPS)].copy()
    df = df.reset_index(drop=True)
    df["EpisodeOrder"] = np.arange(1, len(df) + 1)
    return df

dfs = {}
for v, p in CSV_FILES.items():
    if p is None:
        print(f"[WARN] {v} 파일 없음 — 건너뜀")
        continue
    dfs[v] = load_clean(p)
    print(f"  {v}: {len(dfs[v])} natural episodes")

if not dfs:
    raise SystemExit("CSV 파일을 찾지 못했습니다. LOG_DIR 경로 확인하세요.")

# ---------- Figure 1: 학습 곡선 (Return) ----------
fig, ax = plt.subplots(figsize=(8, 5))
for v, df in dfs.items():
    smoothed = df["Return"].rolling(SMOOTH_WINDOW, min_periods=1).mean()
    ax.plot(df["EpisodeOrder"], smoothed,
            label=VARIANT_LABELS[v], color=VARIANT_COLORS[v], linewidth=2)
ax.set_xlabel("Episode (natural completions)")
ax.set_ylabel(f"Return (moving avg, window={SMOOTH_WINDOW})")
ax.set_title("Learning Curves by Reward Variant")
ax.legend(loc="lower right")
ax.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(OUT_DIR / "learning_curves.png", dpi=150)
print(f"\nSaved: {OUT_DIR / 'learning_curves.png'}")

# ---------- Figure 2: 누적 성공률 ----------
fig, ax = plt.subplots(figsize=(8, 5))
for v, df in dfs.items():
    cumulative_success_rate = df["Success"].expanding().mean() * 100
    ax.plot(df["EpisodeOrder"], cumulative_success_rate,
            label=VARIANT_LABELS[v], color=VARIANT_COLORS[v], linewidth=2)
ax.set_xlabel("Episode (natural completions)")
ax.set_ylabel("Cumulative Success Rate (%)")
ax.set_title("Cumulative Success Rate by Reward Variant")
ax.legend(loc="lower right")
ax.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(OUT_DIR / "success_rate.png", dpi=150)
print(f"Saved: {OUT_DIR / 'success_rate.png'}")

# ---------- Figure 3: Rolling 성공률 (window=100) ----------
fig, ax = plt.subplots(figsize=(8, 5))
for v, df in dfs.items():
    rolling_sr = df["Success"].rolling(100, min_periods=1).mean() * 100
    ax.plot(df["EpisodeOrder"], rolling_sr,
            label=VARIANT_LABELS[v], color=VARIANT_COLORS[v], linewidth=2)
ax.set_xlabel("Episode (natural completions)")
ax.set_ylabel("Success Rate (rolling, window=100, %)")
ax.set_title("Rolling Success Rate by Reward Variant")
ax.legend(loc="lower right")
ax.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(OUT_DIR / "rolling_success_rate.png", dpi=150)
print(f"Saved: {OUT_DIR / 'rolling_success_rate.png'}")

# ---------- Table 1: 요약 통계 ----------
rows = []
for v, df in dfs.items():
    successes = df[df["Success"] == 1]
    first_success_ep = int(successes["EpisodeOrder"].iloc[0]) if len(successes) > 0 else None
    final_window = df.tail(FINAL_TAIL)
    final_sr = final_window["Success"].mean() * 100
    successful_steps = successes["Steps"]
    mean_success_steps = float(successful_steps.mean()) if len(successful_steps) > 0 else None

    rows.append({
        "Variant": VARIANT_LABELS[v],
        "Total Episodes": len(df),
        "Total Successes": int(df["Success"].sum()),
        "Overall Success Rate (%)": round(df["Success"].mean() * 100, 2),
        f"Final {FINAL_TAIL}ep Success Rate (%)": round(final_sr, 2),
        "First Success Episode": first_success_ep if first_success_ep else "—",
        "Mean Steps (Success only)": round(mean_success_steps, 1) if mean_success_steps else "—",
        "Mean Return": round(df["Return"].mean(), 3),
    })

summary_df = pd.DataFrame(rows)
summary_df.to_csv(OUT_DIR / "summary_table.csv", index=False, encoding="utf-8-sig")
print(f"Saved: {OUT_DIR / 'summary_table.csv'}")

print("\n=== Summary Table ===")
print(summary_df.to_string(index=False))

# ---------- Markdown 표 (논문 붙여넣기용) ----------
md_table = summary_df.to_markdown(index=False)
(OUT_DIR / "summary_table.md").write_text(md_table, encoding="utf-8")
print(f"\nMarkdown table:\n{md_table}")
print(f"\nSaved: {OUT_DIR / 'summary_table.md'}")
