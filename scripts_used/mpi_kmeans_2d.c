#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

void print_points(double** points, int num_points, int input_dim) {

	printf("heeeere\n");
	printf("%d, %d\n", num_points, input_dim);
	int i, j;

	for (i=0; i<num_points; i++) {
		printf("( ");
		for (j=0; j<input_dim; j++) {
			fprintf(stdout, "%lf ", points[i][j]);
		}
		printf(" )\n");
	}
}

void find_centroids(double** centroids, int num_clusters, double** points, int* membership, int num_points, int input_dim) {
	int i, j;

	for (i=0; i<num_clusters; ++i) {
		for (j=0; j<input_dim; ++j) {
			centroids[i][j] = 0;
		}
	}

	int* counts = (int*)calloc(num_clusters, sizeof(int));

	for (i=0; i<num_points; ++i) {
		for (j=0; j<input_dim; ++j) {
			centroids[membership[i]-1][j] += points[i][j];
		}
		counts[membership[i]-1]++;
	}

	for (i=0; i<num_clusters; ++i) {
		for (j=0; j<input_dim; ++j) {
			if (counts[i] != 0) {
				centroids[i][j] /= counts[i];
			}
		}
	}
}

int num_changed_members(int* membership_old, int* membership_new, int num_points) {
	int i;
	int num_changed = 0;
	for (i=0; i<num_points; ++i) {
		if (membership_old[i] != membership_new[i]) {
			num_changed++;
		}
	}

	return num_changed;
}

void init_cluster_centroids(int* centroids, int num_clusters, int num_points) {
	srand(time(NULL));

	int i;
	int* points_shuffle = (int*)malloc(num_points*sizeof(int));
	for (i=0; i<num_points; i++) {
		points_shuffle[i] = i;
	}

	for (i=0; i<num_points; i++) {
		int temp = points_shuffle[i];
		int randomIndex = rand()%num_points;

		points_shuffle[i] = points_shuffle[randomIndex];
		points_shuffle[randomIndex] = temp;
	}

	for (i=0; i<num_clusters; ++i) {
		centroids[i] = points_shuffle[i];
	}
}

double distance(double* x, double* y, int input_dim) {
	/* Squared Euclidean, but can be changed to anything */
	int i;
	double dist = 0;

	for (i=0; i<input_dim; i++) {
		dist += (x[i] - y[i])*(x[i] - y[i]);
	}

	return dist;
}

int main(int argc, char **argv) {
	
	if (argc < 4) {
		printf("Usage: %s <input_path> <input_dimensions> <num_clusters>\n", argv[0]);
		exit(1);
	}

	int numtasks,              /* number of tasks in partition */
		taskid,                /* a task identifier */
		numworkers,            /* number of worker tasks */
	    source,                /* task id of message source */
		dest,                  /* task id of message destination */
		mtype,                 /* message type */
		rows,                  /* rows of 'points array' sent to each worker */
		averow, extra, offset, /* used to determine points sent to each worker */
		i, j, k, rc;           /* misc */

	MPI_Status status;

	/* Parse dimension of input */
	int input_dim = atoi(argv[2]);

	/* Parse number of clusters */
	int num_clusters = atoi(argv[3]);

	/* Cluster specific data structures */
	int num_points; 		//no. of points in dataset
	double **points;		//data structure for points
	int *membership_new;	//membership for clusters
	double **centroids = (double**)malloc(num_clusters*sizeof(double*)); //centroids of clusters

	/* MPI Initialization */
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	if (numtasks < 2 ) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}
	numworkers = numtasks-1;

	if(taskid == MASTER) {
		printf("mpi_kmeans has started with %d tasks.\n", numtasks);
		
		FILE *fp = fopen(argv[1], "r");
		num_points = 0;

		/* Going through the file to find the number of points */
		while (!feof(fp)) {
			double t1, t2;
			fscanf(fp, "%lf,%lf\n", &t1, &t2);
			num_points++;
		}

		/* Initialize array for points */
		points = (double**)malloc(num_points*sizeof(double*));
		int i;
		for (i=0; i<num_points; ++i) {
			points[i] = (double*)malloc(input_dim*sizeof(double));
		}

		/* Send file pointer to beginning of file */
		rewind(fp);
	
		/* Read the points into an array */
		i = 0;
		int j = 0;
		while (!feof(fp)) {
			for (j=0; j<input_dim; ++j) {
				if (j == 0)
					fscanf(fp, "%lf", &points[i][j]);
				else
					fscanf(fp, ",%lf", &points[i][j]);
			}
			fscanf(fp, "\n");
			i++;
		}

		//print_points(points, num_points, input_dim);
	
		/* Arrays to keep track of point memberships */
		int *membership_old = (int*)malloc(num_points*sizeof(int));
		membership_new = (int*)malloc(num_points*sizeof(int));

		/* initialize to dummy cluster values */
		for (i=0; i<num_points; ++i) {
			membership_new[i] = 1;
		}

		/* Initialize cluster centroids */
		int *centroid_idx = (int*)malloc(num_clusters*sizeof(int));
		init_cluster_centroids(centroid_idx, num_clusters, num_points);

		//centroids = (double**)malloc(num_clusters*sizeof(double*));
		for (i=0; i<num_clusters; ++i) {
			centroids[i] = (double*)malloc(input_dim*sizeof(double));
			for (j=0; j<input_dim; ++j) {
				centroids[i][j] = points[centroid_idx[i]][j];
			}
		}

		print_points(centroids, num_clusters, input_dim);

		/* The K-means loop */
		int num_iter = 0;
		do {
			num_iter++;
			for (i=0; i<num_points; i++) {
				membership_old[i] = membership_new[i];
			}

			averow = num_points/numworkers;
			extra = num_points%numworkers;
			offset = 0;
			mtype = FROM_MASTER;
			for (dest=1; dest<=numworkers; dest++){
				rows = (dest <= extra) ? averow+1 : averow;
				printf("Sending %d points to task %d offset=%d\n",rows,dest,offset);
				//MPI_Send(&num_points, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
				MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
				MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
				MPI_Send(&points[offset][0], rows*input_dim, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
				MPI_Send(centroids, num_clusters*input_dim, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
				MPI_Send(&membership_new[offset], rows, MPI_INT, dest, mtype, MPI_COMM_WORLD);
				offset = offset + rows;
			}


			/* Assign points to clusters */
			for (i=0; i<num_points; ++i) {
				double min_dist = distance(points[i], centroids[0], input_dim);
				int min_dist_idx = 1;
				/* For each point */
				for (j=0; j<num_clusters; ++j) {
					/* For each cluster, find distance */
					double dist = distance(points[i], centroids[j], input_dim);
				
					if (dist < min_dist) {
						min_dist_idx = j+1;
						min_dist = dist;
					}
				}

				/* Give the point its new cluster */
				membership_new[i] = min_dist_idx;
			}

			find_centroids(centroids, num_clusters, points, membership_new, num_points, input_dim);

		} while (num_changed_members(membership_old, membership_new, num_points) != 0);
	
		printf("Finished in %d iterations\n", num_iter);
		printf("The final centroids are:\n");
		print_points(centroids, num_clusters, input_dim);
	}

	if(taskid > MASTER) {
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		
		double** w_points = (double**)malloc(rows*sizeof(double *));
		int i;
		for(i=0; i<rows; i++) {
			w_points[i] = (double*)malloc(input_dim*sizeof(double));
		}
		printf("rows recd\n");
		printf("rows: %d\n", input_dim);
		
		MPI_Recv(&w_points, rows*input_dim, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

		printf("rows: %p\n", w_points);
		print_points(w_points, rows, input_dim);
		
		double** w_centroids = (double**)malloc(num_clusters*sizeof(double *));
		for(i=0; i<num_clusters; i++) {
			w_centroids[i] = (double*)malloc(input_dim*sizeof(double));
		}
		printf("points recd\n");
		
		MPI_Recv(&w_centroids, num_clusters*input_dim, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
	
		printf("centroids recd\n");
		int* w_membership_new = (int*)malloc(rows*sizeof(int));
		
		MPI_Recv(&w_membership_new, rows, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);

		printf("end\n");
	}

	MPI_Finalize();

	return 0;
}
