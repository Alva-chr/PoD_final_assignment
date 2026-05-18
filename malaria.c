#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "prop.h"


int main(int argc, char **argv) {

	if (2 != argc) {
		printf("Usage: total number of simulations run\n");
		return 1;
	}
    int N = atoi(argv[1]); //total number of process simulations to run

    int T = 100; //Timesteps simulation will take
    int t = 0; //start time

	int rank, size;
	MPI_Status status;

	//Setting up MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(N%size != 0){
        printf("ERROR: Total number of simulations, %d, must be divisable by the number of process, %d.", N,size);
        return -1;
    }

    int n = N/size; //simulations run per process
    int x0 = [900, 900, 30, 330, 50, 270, 20];
    int x = [0, 0, 0, 0, 0, 0, 0];
    double w = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    int* process_memory = malloc((7*n)*sizeof(double));
    int collected_data = malloc((7*n*size)*sizeof(double)); // for collecting the data in the end
    int simulations_done = 0;

	//for time taking
	int T = 100;
	int t = 0;

	//for random variables
	double u1, u2;
	double tau; //timestep
	double a0u2;
	double r;


	double a0 = 0

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
			w = prop(&x,&w);
            
			//Step 2 in SSA
			for(i=0, i <15, i++){
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
		simulations_done +=1;
    }
}

int write_output(char *file_name, const double *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4f ", output[i])) {
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