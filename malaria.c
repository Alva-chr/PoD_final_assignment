#include <string.h>
#include <stdio.h>


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
    double x0 = [900, 900, 30, 330, 50, 270, 20];
    double w = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
    int* process_memory = malloc((7*n)*sizeof(double));
    int simulations_done = 0;

    //running all the simulations
    while(simulations_done < n){
        //One simulation run
        for(t=0; t<T; t ++){

        }

    }






	
}


int read_input(const char *file_name, double **values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "r"))) {
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values)) {
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double)))) {
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i=0; i<num_values; i++) {
		if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file)){
		perror("Warning: couldn't close input file");
	}
	return num_values;
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