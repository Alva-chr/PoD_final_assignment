#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "prop.h"

//modified from precvius assignments
int write_output(char *file_name, const int *output, const int *bin_Sizes, int num_values, int bin_size) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < 20; i++) {
		if (0 > fprintf(file, "%d\n", bin_Sizes[i])) {
			perror("Couldn't write to output file");
		}
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%d\n", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}


int main(int argc, char **argv) {

	if (3 != argc) {
		printf("Usage: total number of simulations run\n");
		return 1;
	}
    int N = atoi(argv[1]); //total number of process simulations to run
	char *output_name = "output.txt";

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
	//setting the random seed
	srand(time(NULL)+rank);

    int n = N/size; //simulations run per process
    int x0[7] = {900, 900, 30, 330, 50, 270, 20};
    int x[7] = {0};
    double w[15] = {0};
    int* process_memory = malloc((7*n)*sizeof(int));

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

    //running all the simulations
    while(simulations_done < n){

		//resetting simulation
        memcpy(&x, &x0, 7*sizeof(int));
		t = 0;

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
	//and then broadcasting it
	MPI_Reduce(&local_max_X, &global_max_X,1, MPI_INT, MPI_MAX,0, MPI_COMM_WORLD);
	MPI_Bcast(&global_max_X, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Reduce(&local_min_X, &global_min_X,1, MPI_INT, MPI_MIN,0, MPI_COMM_WORLD);
	MPI_Bcast(&global_min_X, 1, MPI_INT, 0, MPI_COMM_WORLD);


	//AN OLD SOLUTION, REMOVE BEFORE TURNIN
	// //Need to determine global max of X so all the proccess have
	// //the same size for the bins
	// MPI_Gather(&local_max_X, 1, MPI_INT, &array_max_X,1, MPI_INT, 0, MPI_COMM_WORLD);

	// //finding the global max X on root
	// if(rank == 0){
	// 	for(i = 0; i <size; i++){
	// 		if(global_max_X <array_max_X[i]){
	// 			global_max_X = array_max_X[i];
	// 		}
	// 	}
	// }

	// //broadcasting the global max X found
	// MPI_Bcast(&global_max_X, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//finding the intervalls for the bins
	// printf("global max X: %d\n",global_max_X);
	// printf("global min X: %d\n", global_min_X);
	int bin_size = (global_max_X-global_min_X)/20;
	// printf("rank %d has bin size of %d\n", rank, bin_size);
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
			//printf("x0 värden: %d\n", all_X0[i]);
			if(bins[j]>all_X0[i] && all_X0[i] <= bins[j+1]){
				local_bins[j] += 1;
				break;
				// if(rank==0){
				//    printf("local bins value: %d\n", local_bins[j]);
				// }
			}
		}
	}

	int global_bins[20] = {0};
	// if(rank == 1){
	// 	for(int i = 0; i <20; i++){
	// 		printf("%d\n", local_bins[i]);
	// 	}
	// }

	MPI_Reduce(local_bins, global_bins, 20, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


	if(rank == 0){
		write_output(output_name, global_bins, bins, 20, bin_size);
	}

	MPI_Finalize();

	free(process_memory);
	free(all_X0);
}

