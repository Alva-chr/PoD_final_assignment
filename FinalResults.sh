#!/bin/bash
#SBATCH -A uppmax2026-1-92      # Project ID
#SBATCH -N 1                    # Number of nodes
#SBATCH --ntasks=32             # INCREASED: Max MPI tasks needed for the job
#SBATCH --ntasks-per-node=32    # INCREASED: MPI tasks per node
#SBATCH --cpus-per-task=1
#SBATCH -t 00:40:00             # Time limit (hh:mm:ss)
#SBATCH --overload-allowed           # Optional: Ensures mpirun can flexibly use subsets of the allocation

module load OpenMPI/5.0.8-GCC-14.3.0

make all


# ADDED 16 and 32 to the loop array
PROCESSES=(1 2 4 8 16 32) 
NUMBER_OF_SIMULATIONS=(1 2 3 4 5 6 7 8)
LOG_FILE="benchmark_results.txt"

# Initialize/Clear the log file
echo "Benchmark Results - $(date)" > $LOG_FILE
echo "------------------------------------------------" >> $LOG_FILE

for ps in "${NUMBER_OF_SIMULATIONS[@]}"; do
     for np in "${PROCESSES[@]}"; do
        echo "  Running with $np processes..."
        echo "PARAMS: Problem size=$ps, Procs=$np" >> $LOG_FILE
        #I do this for the sake of averaging the results
        for i in {1..5}; do
            mpirun -n $np ./malaria  $ps  >> $LOG_FILE 2>&1
        done
        echo "------------------------------------" >> $LOG_FILE

    done
done

echo "Benchmarks complete. Results saved to $LOG_FILE"