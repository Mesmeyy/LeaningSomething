#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define ARRAY_SIZE 18


int main(int argc,char* argv[])
{
	int rank;
	int processes;
	int arraysize = ARRAY_SIZE;
	unsigned long array[ARRAY_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	MPI_Bcast(&array,ARRAY_SIZE,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&processes,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&arraysize,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

	int everychoice;

	if(rank == 0) {
		int after = ((rank+1)*arraysize)/processes;
		int before = (rank*arraysize)/processes;
		int thisprocessnumber = after-before;
		int randomnumber = rand() % thisprocessnumber;
		int choicenumber = array[before+randomnumber];
		printf("choicenumber = %d\n",choicenumber);
	}else{
		int after = ((rank+1)*arraysize)/processes;
		int before = (rank*arraysize)/processes;
		int thisprocessnumber = after-before;
		int randomnumber = rand() % thisprocessnumber;
		int choicenumber = array[before+randomnumber];
		printf("choicenumber = %d\n",choicenumber);
	}


	MPI_Finalize();



}
