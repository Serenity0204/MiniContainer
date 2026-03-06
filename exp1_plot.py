import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

targets = [20, 50, 80]
trials = [1, 2, 3]
trial_colors = {1: '#1f77b4', 2: '#ff7f0e', 3: '#2ca02c'} 

trial_linewidths = {1: 5, 2: 2.5, 3: 1} 
trial_alphas = {1: 0.4, 2: 0.8, 3: 1.0}  
trial_linestyles = {1: '-', 2: '--', 3: ':'} 

fig, axes = plt.subplots(len(targets), 1, figsize=(10, 12), sharex=True)

for i, target in enumerate(targets):
    ax = axes[i]
    for trial in trials:
        data_dir = "data/exp1"
        filename = os.path.join(data_dir, f"cpu_{target}_percent_{trial}.csv")
        if os.path.exists(filename):
            df = pd.read_csv(filename)

            cpu_data = df.iloc[:, 0]
            
            time_seconds = np.arange(len(cpu_data))
            
            ax.plot(time_seconds, cpu_data, 
                    label=f"Trial {trial}", 
                    color=trial_colors[trial], 
                    linewidth=trial_linewidths[trial], 
                    alpha=trial_alphas[trial],
                    linestyle=trial_linestyles[trial])

    ax.set_title(f"Target CPU Usage: {target}%", fontsize=14, fontweight='bold')
    ax.set_ylabel("CPU Usage (%)", fontsize=12)
    
    ax.set_ylim(target - 3, target + 3) 
    
    ax.axhline(y=target, color='red', linestyle='-', alpha=0.3, linewidth=1, label='Target')

    ax.legend(loc='upper right', title="Trials")
    ax.grid(True, linestyle='--', alpha=0.7)

plt.xlabel("Time (seconds)", fontsize=12)
plt.suptitle("CPU Usage Trials Comparison", fontsize=16, y=1.02)
plt.tight_layout()

os.makedirs("img", exist_ok=True)
plt.savefig("img/cpu_comparison_plots.png", dpi=300, bbox_inches='tight')
plt.show()