#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "prop.h"

int main(int argc, char **argv) {

	if (3 != argc) {
		printf("Usage: total number of simulations run\n");
		return 1;
	}
    int N = atoi(argv[1]); //total number of process simulations to run
	char * output_name = argv[2];

    double T = 100; //Timesteps simulation will take
    double t = 0; //start time

	int rank, size;

	//Setting up MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Win win;

    if(N%size != 0){
        printf("ERROR: Total number of simulations, %d, must be divisable by the number of process, %d.", N,size);
        return -1;
    }
	//setting the random seed
	srand(rank);

    int n = N/size; //simulations run per process
    int x0[7] = {900, 900, 30, 330, 50, 270, 20};
    int x[7] = {0};
    double w[15] = {0};
    int* process_memory = malloc((7*n)*sizeof(int));

	//variables for finding average walltime to 25, 50, 75 and 100 timesteps.
	double* timings = malloc(4*n*sizeof(double));
	double average_timings[4] = {0};
	int t_count = 0;
	double* collected_timings;

	MPI_Alloc_mem((4*size)*sizeof(double), MPI_INFO_NULL, &collected_timings);
	MPI_Win_create(collected_timings, 4*size*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    int simulations_done = 0;

	//for random variables
	double u1, u2;
	double tau; //timestep
	double a0u2;
	int r = 7;
	int idx;
	double w_ahead, w_behind;

	double a0 = 0;

	int i;

	int P[15][7] = {
		{ 1,  0,  0,  0,  0,  0,  0},
		{-1,  0,  0,  0,  0,  0,  0},
		{-1,  0,  1,  0,  0,  0,  0},
		{ 0,  1,  0,  0,  0,  0,  0},
		{ 0, -1,  0,  0,  0,  0,  0},
		{ 0, -1,  0,  1,  0,  0,  0},
		{ 0,  0, -1,  0,  0,  0,  0},
		{ 0,  0, -1,  0,  1,  0,  0},
		{ 0,  0,  0, -1,  0,  0,  0},
		{ 0,  0,  0, -1,  0,  0,  0},
		{ 1,  0,  0,  0, -1,  0,  0},
		{ 0,  0,  0,  0, -1,  0,  1},
		{ 0,  0,  0,  0,  0, -1,  0},
		{ 1,  0,  0,  0,  0,  0, -1},
		{ 0,  0,  0,  0,  0,  0, -1}
	};

	double start = MPI_Wtime();
	double start_partial;

    //running all the simulations
    while(simulations_done < n){

		//resetting simulation
        memcpy(&x, &x0, 7*sizeof(int));
		t = 0;
		t_count = 0;

		start_partial = MPI_Wtime();
		//time simulations
        //One simulation run
        while(t<T){
			a0 = 0;

			//Step one in SSA
			prop(x,w);
            
			//Step 2 in SSA
			for(i=0; i <15; i++){
				a0 += w[i];
			}

			//Step 3 in SSA
			u1 = (double)rand()/((double)RAND_MAX);
			u2 = (double)rand()/((double)RAND_MAX);

			//Step 4 in SSA
			tau = -1*log(u1)/a0;

			//step 5 in SSA
			a0u2 = a0*u2;

			for(i = 0; i <15;i++){
				w_ahead = 0;
				w_behind = 0;

				//finding the sum of w before the i:th element
				for(int j = 0;j<i;j++){
					w_behind += w[j];
				}
				//finding the sum of w after the i:th element up to the last
				for(int j = i; j<15;j++ ){
					w_ahead += w[j];
				}

				if(w_behind < a0*u2 && a0u2 <= w_ahead){
					r = i;
				}
			}

			//step 5 in SSA
			for(i = 0;i <7;i++){
				x[i] += P[r][i];
			}

			//saving the wall time at the wanted timing intervals
			if((t<25 && t+tau>25)||(t<50 && t+tau>50)||(t<75 && t+tau>75)||(t<100 && t+tau>100)){
				timings[4*simulations_done+t_count] = MPI_Wtime() - start_partial;
				t_count ++;
			}
			//step 6 in SSA
			t += tau;
        }

		//saving the data in each process 
		for(i = 0; i <7;i++){
			idx = 7*simulations_done + i;
			process_memory[idx] = x[i];
		}

		simulations_done +=1;
    }

	//finding the sum of the wall time
	for(i = 0; i <n;i++){
		for (int j=0;j<4;j++){
			average_timings[j] += timings[4*i+j];
		}
	}

	//finding the average
	for(i = 0; i<4; i++){
		average_timings[i] /= n;
	}

	//Using Put/get functionality to save the timings on the root process.
	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0,win);
		MPI_Put(&average_timings, 4, MPI_DOUBLE, 0, 4*rank, 4, MPI_DOUBLE,win);
	MPI_Win_unlock(0, win);

	//Finding the higest value and lowest value of X so all processors have the same size for the bins in the histogram
	int local_max_X = process_memory[0];
	int global_max_X = 0;

	int local_min_X = process_memory[0];
	int global_min_X = 0;

	int *all_X0 = malloc(n*sizeof(int)); 
	for(i = 0; i<n; i++){
		idx = 7*i;
		//finding local max
		if(local_max_X < process_memory[idx]){
			local_max_X = process_memory[idx];
		}

		if(local_min_X > process_memory[idx]){
			local_min_X = process_memory[idx];
		}

		all_X0 [i] = process_memory[idx];
	}

	//finding the minimum and maximum value of X in all process 
	MPI_Allreduce(&local_max_X, &global_max_X,1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	MPI_Allreduce(&local_min_X, &global_min_X,1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

	//finding the intervalls for the bins
	int bin_size = (global_max_X-global_min_X)/20;
	int bins[21] = {0};
	bins[0] = global_min_X;
	bins[20] = global_max_X;
	for (i = 1; i <20;i++){
		bins[i] = bins[i-1]+bin_size;
	}

	int local_bins[20] = {0};

	//Going over all the x-values and putting them in the correct bin
	for(i = 0; i<n; i++){
		for(int j = 0;j<20;j++){
			if(bins[j]<=all_X0[i] && all_X0[i] < bins[j+1]){
				local_bins[j] += 1;
				break;
			}
			else if(j==19 && all_X0[i] == bins[20]){
				local_bins[j] += 1;
			}
		}
	}

	int global_bins[20] = {0};

	MPI_Reduce(local_bins, global_bins, 20, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	double my_execution_time = MPI_Wtime() - start;
	double max_execution_time = 0;

    //Take the slowest execution time
	MPI_Reduce(&my_execution_time, &max_execution_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	//writing out to the output files
	//one output file for the results
	//one output file for the average time per processor for each time sub-interval
	if(rank == 0){
		printf("%f\n", max_execution_time);

		//outputting the results
		FILE *file1;
		file1 = fopen(output_name, "w");
		fprintf(file1, "%d\n", N);

		for (int i = 0; i < 21; i++) {
			fprintf(file1, "%d\n", bins[i]);
		}
		for (int i = 0; i < 20; i++) {
			fprintf(file1, "%d\n", global_bins[i]);
		}
		fclose(file1);


		//outputting the the average time per processor for each time sub-interval
		FILE *file2;
		file2 = fopen("processor_timings.txt", "w");

		for (int i = 0; i < size; i++) {
			fprintf(file2, "%d ", i);

			for(int j = 0; j<4;j++){
				fprintf(file2, "%f ", collected_timings[4*i+j]);
			}
			fprintf(file2, "\n");
		}
		fclose(file2);
	}

	MPI_Win_free(&win);
	MPI_Free_mem(collected_timings);

	MPI_Finalize();

	free(process_memory);
	free(all_X0);
}