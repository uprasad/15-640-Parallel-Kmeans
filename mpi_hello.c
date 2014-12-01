/******************************************************************************
* FILE: mpi_hello.c
* DESCRIPTION:
*   MPI tutorial example code: Simple hello world program
* AUTHOR: Blaise Barney
* LAST REVISED: 03/05/10
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  MASTER		0
#define FROM_MASTER 1

int main (int argc, char *argv[])
{
int   numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];

int i;
for(i=0; i<argc; i++) {
	printf("%s\t", argv[i]);
}
printf("\n");

int *arr = (int*)malloc(30*sizeof(int));
//int arr[30];

int mtype;
MPI_Status status;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);
printf ("Hello from task %d on %s!\n", taskid, hostname);


if (taskid == MASTER) {
	mtype = FROM_MASTER;
   printf("MASTER: Number of MPI tasks is: %d\n",numtasks);
   //arr = (int*)malloc(10*3*sizeof(int));

	int i;
	for (i=0; i<30; ++i) {
		arr[i] = i;
	}

   MPI_Send(&arr[10], 2*3, MPI_INT, 1, mtype, MPI_COMM_WORLD);
}
if (taskid > MASTER) {
	mtype = FROM_MASTER;

	//arr = (int*)malloc(6*sizeof(int));
	//int w_arr[6];

	MPI_Recv(arr, 2*3, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
	
	int i;
	for (i=0; i<6; i++) {
		printf("%d, ", arr[i]);
	}
	printf("\n");
}

MPI_Finalize();

}

