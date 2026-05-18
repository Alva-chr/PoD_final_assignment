#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include "prop.h"

//modified from precvius assignments
int write_output(char *file_name, const int *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4d ", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n")) {
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}


int main(int argc, char **argv) {

	if (3 != argc) {
		printf("Usage: total number of simulations run | output file name\n");
		return 1;
	}
    int N = atoi(argv[1]); //total number of process simulations to run
	char *output_name = argv[2];


    double T = 100; //Timesteps simulation will take
    double t = 0; //start time

	int rank, size;

	//Setting up MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(N%size != 0){
        printf("ERROR: Total number of simulations, %d, must be divisable by the number of process, %d.", N,size);
        return -1;
    }

    int n = N/size; //simulations run per process
    int x0[7] = {900, 900, 30, 330, 50, 270, 20};
    int x[7] = {0};
    double w[15] = {0};
    int* process_memory = malloc((7*n)*sizeof(double));

    int simulations_done = 0;

	//for random variables
	double u1, u2;
	double tau; //timestep
	double a0u2;
	int r = 7;
	int idx;


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

    //running all the simulations
    while(simulations_done < n){

		//resetting simulation
        memcpy(&x, &x0, 7*sizeof(int));
		a0 = 0;

		//time simulations
        //One simulation run
        while(t<T){
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
			for(i = 1; i <15;i++){
				if(w[i-1] < a0*u2 && a0u2 <= w[i]){
					r = i;
				}
			}

			//step 5 in SSA
			for(i = 0;i <7;i++){
				x[i] += P[r][i];
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

	//Collecting only the relevant data
	int local_max_X = 0;
	int array_max_X[size];
	int global_max_X;
	int *all_X0 = malloc(n*sizeof(int)); 
	for(i = 0; i<n; i++){
		idx = 7*i;
		if(local_max_X < process_memory[idx]){
			local_max_X = process_memory[idx];
		}

		all_X0 [i] = process_memory[idx];
	}

	//Need to determine global max of X so all the proccess have
	//the same size for the bins
	MPI_Gather(&local_max_X, 1, MPI_INT, &array_max_X,1, MPI_INT, 0, MPI_COMM_WORLD);

	//finding the global max X on root
	if(rank == 0){
		for(i = 0; i <size; i++){
			if(global_max_X <array_max_X[i]){
				global_max_X = array_max_X[i];
			}
		}
	}

	//broadcasting the global max X found
	MPI_Bcast(&global_max_X, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//finding the intervalls for the bins
	int bin_size = global_max_X/20;
	int local_bins[20] = {0};

	//Going over all the x-values and putting them in the correct bin
	for(i = 0; i<n; i++){
		for(int j = 0;j<20;j++){
			if(bin_size*(j)<all_X0[i] && all_X0[i] <= bin_size*(j+1)){
				local_bins[j] += 1;
			}
		}
	}

	int global_bins[20] = {0};

	MPI_Reduce(local_bins, global_bins, 20, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


	if(rank == 0){
		write_output(output_name, global_bins, 20);
	}

	MPI_Finalize();

	free(process_memory);
	free(all_X0);
}

