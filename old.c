	//AN OLD SOLUTION, REMOVE BEFORE TURNIN
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