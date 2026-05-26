This program simulates a malaria epidemic by SSA algorithm and extracts
the first element in the state-vector which is the number of susceptible people.
The program is parallelized using openMPI and the problem sizes are assumed to be
divisble by the number of process used. 

N = n * p

where N is the total problem size, n is the amount of simulations
each processor will run and p is the number of processor.

Run the following commands to run the program

**First load the correct MPI**
module load OpenMPI/5.0.8-GCC-14.3.0

**Then compile the file**
Make all

**Then run the command**
mpirun -n $np --mca osc ^ucx ./malaria  $ps $out_file

*$np: number of process*
*$ps: the problem size*
*$out_file: is the name of the output file*

The output in $out_file will be saved as a txt where the first line
is the problem size, the next 21 lines are the bin intervals and the last 20 are the amount of simulations per bin.

The output of the average wall clock time per processor for each time sub-interval is written to the file called processor_timings.txt where to first column is the processor, the second column is the timing for the 25 timestep, the third column is the timings for the 75 timesteps and the last column are the timings for 100 time steps.