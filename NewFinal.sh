#!/bin/bash
#SBATCH -A uppmax2026-1-92      # Project ID
#SBATCH -N 1                    # Number of nodes
#SBATCH --ntasks=32             # INCREASED: Max MPI tasks needed for the job
#SBATCH --ntasks-per-node=32    # INCREASED: MPI tasks per node
#SBATCH --cpus-per-task=1
#SBATCH -t 11:00:00             # Time limit (hh:mm:ss)

module load OpenMPI/5.0.8-GCC-14.3.0

make all


# ADDED 16 and 32 to the loop array
PROCESSES=(8) 
NUMBER_OF_SIMULATIONS=(1000000)
LOG_FILE="forProcess.txt"

# Initialize/Clear the log file
echo "Benchmark Results - $(date)" > $LOG_FILE
echo "------------------------------------------------" >> $LOG_FILE

for idx in "${!NUMBER_OF_SIMULATIONS[@]}"; do
    ps="${NUMBER_OF_SIMULATIONS[$idx]}"
     for np in "${PROCESSES[@]}"; do
        out_file="size${ps}_p${np}.txt"
        echo "  Running with $np processes, problem size = $ps..."
        echo "PARAMS: Problem size=$ps, Procs=$np" >> $LOG_FILE
        #I do this for the sake of averaging the results
        for i in {1..3}; do
            mpirun -n $np --mca osc ^ucx ./malaria  $ps $out_file >> $LOG_FILE 2>&1
        done
        echo "------------------------------------" >> $LOG_FILE

    done
done

echo "Benchmarks complete. Results saved to $LOG_FILE"